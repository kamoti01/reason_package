/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A 3D-scope

  Copyright (c) 1997-2000 Mark Danks. mark@danks.org
  Copyright (c) G�nther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef INCLUDE_SCOPEXYZ_TILDE_H_
#define INCLUDE_SCOPEXYZ_TILDE_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  scopeXYZ
    
  Creates a scopeXYZ~

  KEYWORDS
  geo
    
  DESCRIPTION

  Inlet~ for signal X
  Inlet~ for signal Y
  Inlet~ for signal Z
     
  -----------------------------------------------------------------*/
class GEM_EXTERN scopeXYZ : public GemShape
{
  CPPEXTERN_HEADER(scopeXYZ, GemShape)

    public:

  //////////
  // Constructor
  scopeXYZ(t_floatarg len=0);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~scopeXYZ();

  //////////
  // Do the renderShapeing
  virtual void 	renderShape(GemState *state);

  //////////
  // The width of the lines in line draw mode
  void	    	linewidthMess(float linewidth);
    	
  //////////
  // How the object should be drawn
  virtual void	typeMess(t_symbol *type);
  
  //-----------------------------------
  // GROUP:	Member variables
  //-----------------------------------
        
  //////////
  // The drawing style (GL_LINE, GL_QUAD_STRIP, etc)
  GLenum	    	m_drawType;
		
  //////////
  // the number of points
  unsigned int m_requestedLength;
  unsigned int m_length;
  virtual void	doLengthMess(int length=0);
  virtual void	lengthMess(int length);

  virtual void	bangMess();

  int m_position; 
    	
  //////////
  // The vertices
  float  *m_vertices;

  t_inlet*m_inX, *m_inY, *m_inZ;


  void perform(t_int count, t_sample*X, t_sample*Y, t_sample*Z);

 private:
    	    
  //////////
  // Static member functions
  static void 	linewidthMessCallback(void *data, t_floatarg linewidth);
  static void 	lengthMessCallback(void *data, t_floatarg linewidth);
  static void 	bangCallback(void *data);

  static void 	dspCallback(void *data, t_signal** sp);
  static t_int*   perform(t_int* w);

};

#endif	// for header file