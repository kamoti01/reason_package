////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyleft  (l) 2002 IOhannes m zm�lnig
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

// we want our pd-class "pix_buffer_class" to be defined not-static
// so other pix_buffer_...-objects can bind to it
#define NO_STATIC_CLASS

#include "pix_buffer.h"
#include "Base/GemPixImageLoad.h"
#include "Base/GemPixImageSave.h"

#include <string.h>
#include <stdio.h>

/////////////////////////////////////////////////////////
//
// pix_buffer
//
/////////////////////////////////////////////////////////

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_buffer, t_symbol*,A_DEFSYM,t_float,A_FLOAT)

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_buffer :: pix_buffer(t_symbol *s,t_float f=100.0)
{
  if (s==&s_){
    static int buffercounter=0;
    char cbuf[16];
    sprintf(cbuf, "pix_buffer_%6d", buffercounter++);
    s=gensym(cbuf);
  }

  if (f<0)f=DEFAULT_NUM_FRAMES;
  m_bindname = s;
  m_numframes = (int)f;
  m_buffer = new imageStruct[m_numframes];
  
  pd_bind(&this->x_obj->ob_pd, m_bindname); 
  outlet_new(this->x_obj, &s_float);
}
/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_buffer :: ~pix_buffer()
{
  if(m_buffer)delete [] m_buffer;
  pd_unbind(&this->x_obj->ob_pd, m_bindname);
}
/////////////////////////////////////////////////////////
// allocateMess
//   allocate memory for m_numframes images of size x*y (with pixelsize=c)
//
/////////////////////////////////////////////////////////
void pix_buffer :: allocateMess(int x, int y, int c)
{
  int i = m_numframes;
  while(i--){
    m_buffer[i].xsize=x;
    m_buffer[i].ysize=y;
    m_buffer[i].csize=c;
    m_buffer[i].format=0;
    m_buffer[i].reallocate();
  }    
}
/////////////////////////////////////////////////////////
// query the number of frames in the buffer
//
/////////////////////////////////////////////////////////
void pix_buffer :: bangMess()
{
  outlet_float(this->x_obj->ob_outlet, m_numframes);
}
int pix_buffer :: numFrames()
{
  return m_numframes;
}
/////////////////////////////////////////////////////////
// put an image into the buffer @ position <pos>
//
/////////////////////////////////////////////////////////
bool pix_buffer :: putMess(imageStruct*img,int pos){
  if (pos<0 || pos>=m_numframes)return false;
  if(!img)return false;
  img->copy2Image(m_buffer+pos);
  return true;
}
/////////////////////////////////////////////////////////
// get an image from the buffer @ position <pos>
//
/////////////////////////////////////////////////////////
imageStruct*pix_buffer :: getMess(int pos){
  if (pos<0 || pos>=m_numframes)return 0;
  return (m_buffer+pos);
}


/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void pix_buffer :: openMess(t_symbol *filename, int pos)
{
  // muss i wie in pix_image die ganzen andern Sachen a machen ????

  // load an image into mem
  char buf[MAXPDSTRING];
  imageStruct *image = NULL;

  // some checks
  if (pos<0 || pos>=m_numframes)
  {
    post("pix_buffer: index %d out of range (0..%d)!", pos, m_numframes);
    return;
  }

  canvas_makefilename(getCanvas(), filename->s_name, buf, MAXPDSTRING);
  image = image2mem(buf);
  if(!image)
  {
    post("pix_buffer: no valid image!");
    return;
  }

  putMess(image,pos);

  // destroy the image-data
  delete image;
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void pix_buffer :: saveMess(t_symbol *filename, int pos)
{
  // load an image into mem
  char buf[MAXPDSTRING];

  imageStruct*img=NULL;

  if(NULL==filename||NULL==filename->s_name||gensym("")==filename){
    error("pix_buffer: no filename given!");
    return;
  }
  img=getMess(pos);
  
  if(img && img->data){
    mem2image(img, filename->s_name, 0);
  } else {
    error("pix_buffer: index %d out of range (0..%d) or slot empty!", pos, m_numframes);
    return;
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_buffer :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator((t_newmethod)_classpix_buffer,gensym("pix_depot"),A_DEFSYM,A_DEFFLOAT,A_NULL);
  class_addmethod(classPtr, (t_method)&pix_buffer::allocateMessCallback,
  		  gensym("allocate"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addbang(classPtr, (t_method)&pix_buffer::bangMessCallback);
  class_addmethod(classPtr, (t_method)&pix_buffer::openMessCallback,
  		  gensym("open"), A_SYMBOL, A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_buffer::saveMessCallback,
  		  gensym("save"), A_SYMBOL, A_FLOAT, A_NULL);
}
void pix_buffer :: allocateMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg c=4)
{
  if (x<1 || y<1 || c<0){
    GetMyClass(data)->error("init-specs out of range");
    return;
  }
  if (c==0)c=4;
  GetMyClass(data)->allocateMess((int)x, (int)y, (int)c);
}
void pix_buffer :: bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
void pix_buffer :: openMessCallback(void *data, t_symbol *filename, t_floatarg pos)
{
  GetMyClass(data)->openMess(filename, (int)pos);
}
void pix_buffer :: saveMessCallback(void *data, t_symbol *filename, t_floatarg pos)
{
  GetMyClass(data)->saveMess(filename, (int)pos);
}
