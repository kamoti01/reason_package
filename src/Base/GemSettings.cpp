////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2009 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// load settings from a file (or get them via env-variables)
//
/////////////////////////////////////////////////////////

#include "Base/GemConfig.h"
#include "GemSettings.h"

#include <map>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif
#ifdef _WIN32
# include <windows.h>
#endif

#define GEM_SETTINGS_FILE "gem.conf"
static const char*s_configdir[] = { 
#ifdef __linux__
  "/etc/pd",
  "~/.pd", 
#elif defined __APPLE__
  "/Library/Pd",
  "~/Library/Pd",
#elif defined  _WIN32
  "%CommonProgramFiles%\\Pd",
  "%AppData%\\Pd",
#endif
  0 /* $(pwd)/gem.conf */
};

class GemSettingsData {
  friend class GemSettings;
 public:
  GemSettingsData(void);
  ~GemSettingsData(void);

 protected:
  // dictionary for setting values
  std::map <t_symbol*, t_atom*> data;

  virtual t_atom*get(t_symbol*name) {
    return data[name];
  }
  virtual void set(t_symbol*name, t_atom*value) {
    // LATER: we should expand envvariables

    t_atom*a=(t_atom*)getbytes(sizeof(t_atom));
    memcpy(a, value, sizeof(t_atom));
    data[name]=a;
  }
  t_symbol*expandEnv(t_symbol*);

  void setEnv(t_symbol*name, const char*env);

  bool open(const char*filename, const char*dirname=NULL) {
    t_binbuf*bb=binbuf_new();
    int r=0;
    if(NULL==filename)
      return false;


    if(dirname) {
      r=binbuf_read(bb, (char*)filename, expandEnv(gensym(dirname))->s_name, 1);
    } else {
      r=binbuf_read_via_path(bb, (char*)filename, (char*)".", 1);
    }

    if(r){
      binbuf_free(bb);
      return false;
    }

    int ac=binbuf_getnatom(bb);
    t_atom*av=binbuf_getvec(bb);

    t_symbol*s=NULL;
    t_atom*a;
    int state=0; /* 0=(next is ID); 1=(next is value); 2=(next is ignored) */

    while(ac--) {
      if (av->a_type == A_SEMI) {
        // done
        if(NULL!=s) {
          set(s, a);
        }
        state=0;
        s=NULL;
      } else {
        switch (state) {
        case 0:
          s=atom_getsymbol(av);
          state=1;
          break;
        case 1:
          a=av;
          state=2;
          break;
        default:
          break;
        }
      }
      av++;
    }

    binbuf_free(bb);
    return true;
  }

  void print(void) {
    std::map <t_symbol*, t_atom*>::iterator it;
    for(it = data.begin(); 
        it != data.end();
        it++)
      {
        startpost("key ['%s']: ", it->first->s_name);
        postatom(1, it->second);
        endpost();
      }
  }
};


GemSettingsData::GemSettingsData(void)
{
  int i=0;
  while(s_configdir[i]) {
    open(GEM_SETTINGS_FILE, s_configdir[i]);
    i++;
  }
  open(GEM_SETTINGS_FILE);


  /* legacy settings via environmental variables */
  setEnv(gensym("texture.rectangle"), "GEM_RECTANGLE_TEXTURE");
  setEnv(gensym("singlecontext"), "GEM_SINGLE_CONTEXT");
  setEnv(gensym("font.face"), "GEM_DEFAULT_FONT");

  print();
}

GemSettingsData::~GemSettingsData(void) {

}

void GemSettingsData::setEnv(t_symbol*key, const char*env) {
  if(NULL==env)return;
  if(NULL==key)return;

  char*result=getenv(env);
  if(NULL==result) {
    return;
  }

  t_atom a;
  errno=0;

  /* try integer */
  long l=strtol(result, NULL, 0);
  if(0==errno) {
    SETFLOAT(&a, l);
    set(key, &a);
  }

  /* try float */
  double d=strtod(result, NULL);
  if(0==errno) {
    SETFLOAT(&a, d);
    set(key, &a);
  }

  /* try symbol */
  SETSYMBOL(&a, gensym(result));
  set(key, &a);

  // we ignore lists and other complex things for now
}

t_symbol*GemSettingsData::expandEnv(t_symbol*value) {
  if(NULL==value)
    return NULL;
  verbose(2, "expanding '%s'", value->s_name);

#ifdef HAVE_WORDEXP_H
  wordexp_t pwordexp;
  wordexp(value->s_name, &pwordexp, 0);
  if(pwordexp.we_wordc) {
    // we only take the first match into account 
    value=gensym(pwordexp.we_wordv[0]);
  }
  wordfree(&pwordexp);
#endif
#ifdef _WIN32
  char bashBuffer[MAX_PATH];
# if 0
  // should we actually bash '/' to '\'?
  char envVarBuffer[MAX_PATH];
  sys_bashfilename(value->s_name, bashBuffer);
	ExpandEnvironmentStrings(bashBuffer, envVarBuffer, MAX_PATH - 2);
# else
	ExpandEnvironmentStrings(value->s_name, envVarBuffer, MAX_PATH - 2);
# endif
  value=gensym(envVarBuffer);
#endif

  verbose(2, "expanded '%s'", value->s_name);
  return value;
}

/* GemSettings: the public API */

GemSettingsData*GemSettings::settings=NULL;


/* public static functions */
void GemSettings::init() {
  settings=new GemSettingsData(); 
}

t_atom*GemSettings::get(t_symbol*s) {
  if(NULL==settings) init();
  settings->get(s);
}
t_atom*GemSettings::get(const char*s) {
  return get(gensym(s));
}

void GemSettings::set(t_symbol*s, t_atom*v) {
  if(NULL==settings) init();
  settings->set(s, v);
}
void GemSettings::set(const char*s, t_atom*v) {
  set(gensym(s), v);
}