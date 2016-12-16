
#include "ofxCubeMap.h"

//--------------------------------------------------------------
ofxCubeMap::ofxCubeMap()
{
	fov 	= 90.0f;
	nearZ 	= 0.01f;
	farZ 	= 1024.0f;
	
	cubeMapCamerasRenderPosition.set( 0.0f, 0.0f, 0.0f );
	
	setupSkyBoxVertices();
}

//--------------------------------------------------------------
// these should all be the same size and all power of two
void ofxCubeMap::load( string pos_x, string neg_x,
					   string pos_y, string neg_y,
					   string pos_z, string neg_z )
{	
	
	// We don't want the texture border hack to be on
	bool wantsTextureBorderHack = false;
	if( ofIsTextureEdgeHackEnabled() )
	{
		wantsTextureBorderHack = true;
		ofDisableTextureEdgeHack();
		ofLogVerbose() << "ofxCubeMap:loadImages (string version), disabled texture hack, re-enabling when done.";
	}
	
	ofDisableArbTex();
	
	ofImage images[6];	
	bool loaded1 = images[0].load(pos_x);
	bool loaded2 = images[1].load(neg_x);
	bool loaded3 = images[2].load(pos_y);
	bool loaded4 = images[3].load(neg_y);
	bool loaded5 = images[4].load(pos_z);
	bool loaded6 = images[5].load(neg_z);
	
	if( loaded1 && loaded2 && loaded3 && loaded4 && loaded5 && loaded6 ) {}
	else { ofLogError() << "ofxCubeMap: failed to load one of the cubemaps!"; }
	
	init( images[0],
		  images[1],
		  images[2],
		  images[3],
		  images[4],
		  images[5] );
	
	if( wantsTextureBorderHack )
	{
		ofEnableTextureEdgeHack();
	}
	
}

//--------------------------------------------------------------

void ofxCubeMap::init( ofImage pos_x, ofImage neg_x,
					   ofImage pos_y, ofImage neg_y,
					   ofImage pos_z, ofImage neg_z )
{	
	
#ifndef TARGET_OPENGLES
	glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
#endif
	
	//create a texture object
	glGenTextures(1, &textureObjectID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObjectID);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
#ifndef TARGET_OPENGLES	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // GL_TEXTURE_WRAP_R is not in the ES2 header, hmm..
#endif
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	
	
	unsigned char * data_px, * data_nx, * data_py, * data_ny, * data_pz, * data_nz;
	
	size = pos_x.getWidth();
	
	//cout << "Channels " << pos_x.getPixels().getNumChannels() << endl;
	
	data_px = pos_x.getPixels().getData();
	data_py = pos_y.getPixels().getData();
	data_pz = pos_z.getPixels().getData();
	
	data_nx = neg_x.getPixels().getData();
	data_ny = neg_y.getPixels().getData();
	data_nz = neg_z.getPixels().getData();
	
	GLuint pixelFormat = GL_RGB;
	if( pos_x.getPixels().getNumChannels() == 4 ) pixelFormat = GL_RGBA;
	
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_px); // positive x
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_py); // positive y
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_pz); // positive z
	
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_nx); // negative x
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_ny); // negative y
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, size, size, 0, pixelFormat, GL_UNSIGNED_BYTE, data_nz); // negative z
}


//--------------------------------------------------------------
void ofxCubeMap::init( int _size, GLuint _channels, GLuint _storageFormat )
{
	size = _size;
	
	//create a texture object
	glGenTextures(1, &textureObjectID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObjectID);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
#ifndef TARGET_OPENGLES
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // GL_TEXTURE_WRAP_R is not in the ES2 header, hmm..
#endif
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	// set textures
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, _channels, size, size, 0, GL_RGB, _storageFormat, 0);
	}
		
	ofFbo::Settings fboSettings = ofFbo::Settings();
	
	fboSettings.width  = size;
	fboSettings.height = size;

	fboSettings.numColorbuffers = 6; // we intend to attach our own colour buffers
	
	fboSettings.useDepth = true;
	
	fboSettings.textureTarget = GL_TEXTURE_2D;
	
	fbo.allocate( fboSettings );
}


//--------------------------------------------------------------
void ofxCubeMap::beginDrawingInto2D( GLuint _face )
{
	fbo.begin();
	
	// Bind the face we wanted to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _face, textureObjectID, 0 );
	
}

//--------------------------------------------------------------
void ofxCubeMap::endDrawingInto2D()
{
	fbo.end();
}


//--------------------------------------------------------------
void ofxCubeMap::beginDrawingInto3D( GLuint _face )
{
	ofPushView();

	beginDrawingInto2D( _face );	
	
	ofSetMatrixMode( OF_MATRIX_PROJECTION );
	ofLoadMatrix( getProjectionMatrix() );
		
	ofSetMatrixMode( OF_MATRIX_MODELVIEW );
	ofPushMatrix();
	ofLoadMatrix( getLookAtMatrixForFace( _face ) );
	
}

//--------------------------------------------------------------
void ofxCubeMap::endDrawingInto3D()
{
	ofPopView();

	ofPopMatrix();
	
	fbo.end();
}

//--------------------------------------------------------------
void ofxCubeMap::bind( int pos )
{
	boundToTextureUnit = pos;
	
	glActiveTexture( GL_TEXTURE0 + pos );
#ifndef TARGET_OPENGLES
	glEnable( GL_TEXTURE_CUBE_MAP );
#endif
	glBindTexture( GL_TEXTURE_CUBE_MAP, textureObjectID );
}

//--------------------------------------------------------------
void ofxCubeMap::unbind()
{
	glActiveTexture( GL_TEXTURE0 + boundToTextureUnit );

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
#ifndef TARGET_OPENGLES
	glDisable( GL_TEXTURE_CUBE_MAP );
#endif
	glActiveTexture( GL_TEXTURE0 );
}

//--------------------------------------------------------------
void ofxCubeMap::drawSkybox( ofVec3f _pos, float _size )
{
	
	if( !drawCubeMapShader.isLoaded() )
	{
		initShader();
	}
	
	drawCubeMapShader.begin();
	
	bind();
	
		ofBoxPrimitive tmpBox( _size, _size, _size, 1, 1, 1 );
	
		ofPushMatrix();
			ofTranslate( _pos );
			tmpBox.getMesh().draw();
		ofPopMatrix();
	
		// Todo: support drawing the cube map without shaders? If so we need to send over texture coordinates as ofVec3f which ofMesh doesn't support yet
	
		/*
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer( 	3, GL_FLOAT, sizeof(ofVec3f), &cubemapVertices.data()->x );
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 	3, GL_FLOAT, sizeof(ofVec3f), &cubemapTexCoords.data()->x );
	
		ofPushMatrix();
			ofScale( _size, _size, _size );
			glDrawArrays(GL_TRIANGLES, 0, cubemapVertices.size() );
		ofPopMatrix();
		
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		*/
	
	unbind();
	
	drawCubeMapShader.end();
}

//--------------------------------------------------------------
void ofxCubeMap::debugDrawCubemapCameras()
{
	for( int i = 0; i < 6; i++ )
	{
		GLuint face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
		ofMatrix4x4 modelViewProjectionMatrix = getLookAtMatrixForFace( face ) * getProjectionMatrix();
		
		ofPushMatrix();
			
			ofMultMatrix( modelViewProjectionMatrix.getInverse() );
	
			ofNoFill();
		
				// Draw box in camera space, i.e. frustum in world space, box -1, -1, -1 to +1, +1, +1
				ofDrawBox(0, 0, 0, 2.0f);
			
			ofFill();
		
		ofPopMatrix();
		
	}
}

//--------------------------------------------------------------
void ofxCubeMap::debugDrawCubemapFaces( float _faceSize, float _border )
{
	for( int i = 0; i < 6; i++ )
	{
		int tmpX = (i * _faceSize) + (i * _border);
		int tmpY = 0;
		drawFace( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , tmpX, tmpY, _faceSize, _faceSize );
	}
}

//--------------------------------------------------------------
unsigned int ofxCubeMap::getTextureID()
{
	return textureObjectID;
}

//--------------------------------------------------------------
int ofxCubeMap::getWidth()
{
	return size;
}

//--------------------------------------------------------------
int ofxCubeMap::getHeight()
{
	return size;
}

//--------------------------------------------------------------
float ofxCubeMap::getFov()
{
	return fov;
}

//--------------------------------------------------------------
void ofxCubeMap::setFov( float _fov )
{
	fov = _fov;
}

//--------------------------------------------------------------
ofVec2f ofxCubeMap::getNearFar()
{
	return ofVec2f( nearZ, farZ );
}

//--------------------------------------------------------------
void ofxCubeMap::setNearFar( ofVec2f _nearFar )
{
	nearZ = _nearFar.x;
	farZ  = _nearFar.y;
}

//--------------------------------------------------------------
void ofxCubeMap::setPosition( ofVec3f& _pos )
{
	cubeMapCamerasRenderPosition.set( _pos.x, _pos.y, _pos.z );
}

//--------------------------------------------------------------
void ofxCubeMap::setPosition( float _x, float _y, float _z )
{
	cubeMapCamerasRenderPosition.set( _x, _y, _z );
}

//--------------------------------------------------------------
ofVec3f* ofxCubeMap::getPosition()
{
	return &cubeMapCamerasRenderPosition;
}

//--------------------------------------------------------------
ofMatrix4x4 ofxCubeMap::getProjectionMatrix()
{
	ofMatrix4x4 perspectiveMatrix;
	perspectiveMatrix.makePerspectiveMatrix(fov, size/(float)size, nearZ, farZ );
	
	return perspectiveMatrix;
}

//--------------------------------------------------------------
ofMatrix4x4 ofxCubeMap::getLookAtMatrixForFace( GLuint _face )
{
	ofMatrix4x4 lookAt;
	
	switch ( _face )
	{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f(  1.0f,  0.0f,  0.0f), ofVec3f(  0.0f, -1.0f,  0.0f) );
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f( -1.0f,  0.0f,  0.0f), ofVec3f(  0.0f, -1.0f,  0.0f) );
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f(  0.0f,  1.0f,  0.0f), ofVec3f(  0.0f,  0.0f,  1.0f) );
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f(  0.0f, -1.0f,  0.0f), ofVec3f(  0.0f,  0.0f, -1.0f) );
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f(  0.0f,  0.0f,  1.0f), ofVec3f(  0.0f, -1.0f,  0.0f) );
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			lookAt.makeLookAtViewMatrix( ofVec3f( 0.0f, 0.0f, 0.0f), ofVec3f(  0.0f,  0.0f, -1.0f), ofVec3f(  0.0f, -1.0f,  0.0f) );
			break;
		default:
			ofLogError() << "ofxCubeMap::getLookAtMatrixForFace, passed in invalid face.";
			break;
    }
	
	lookAt.glTranslate( -cubeMapCamerasRenderPosition.x, -cubeMapCamerasRenderPosition.y, -cubeMapCamerasRenderPosition.z );
	
	return lookAt;
}


//--------------------------------------------------------------
void ofxCubeMap::drawFace( GLuint _face, float _x, float _y )
{
	drawFace( _face, _x, _y, size, size );
}

//--------------------------------------------------------------
//
//  Used to draw the faces to screen in 2D, usually to debug.
//	The code would look something like:
//
//  for( int i = 0; i < 6; i++ )
//  {
// 	   myFboCubeMap.drawFace( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , i * 100, 0, 100, 100 );
//  }
//
void ofxCubeMap::drawFace( GLuint _face, float _x, float _y, float _w, float _h )
{
	// create a rect with the correct 3D texture coordinates, draw to screen
	scratchVertices.clear();
	scratchTexCoords.clear();
	scratchIndices.clear();
	
	switch ( _face )
	{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			
			scratchTexCoords.push_back( ofVec3f( 1.0f, -1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f( 1.0f,  1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f( 1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( 1.0f, -1.0f, -1.0f) );
			
			break;
			
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f,  1.0f) );
			
			break;
			
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f,  1.0f,  1.0f) );
			
			break;
			
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f, -1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f, -1.0f, -1.0f) );
			
			break;
			
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f,  1.0f,  1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f, -1.0f,  1.0f) );
			
			break;
			
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			
			scratchTexCoords.push_back( ofVec3f(  1.0f, -1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f(  1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f,  1.0f, -1.0f) );
			scratchTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
			
			break;
			
		default:
			
			ofLogError() << "ofxCubeMap::drawFace, passed in invalid face.";
			
			break;
	}
	
	scratchVertices.push_back( ofVec3f( _x, 		_y + _h, 	0.0f ) );
	scratchVertices.push_back( ofVec3f( _x, 		_y, 		0.0f ) );
	scratchVertices.push_back( ofVec3f( _x + _w, 	_y, 		0.0f ) );
	scratchVertices.push_back( ofVec3f( _x + _w, 	_y + _h, 	0.0f ) );
	
	scratchIndices.push_back( 0 );
	scratchIndices.push_back( 1 );
	scratchIndices.push_back( 2 );
	
	scratchIndices.push_back( 0 );
	scratchIndices.push_back( 2 );
	scratchIndices.push_back( 3 );
	
	// Swap all this for an ofMesh when it supports ofVec3f tex coordinates
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer( 	3, GL_FLOAT, sizeof(ofVec3f), &scratchVertices.data()->x );
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer( 	3, GL_FLOAT, sizeof(ofVec3f), &scratchTexCoords.data()->x );
	
	bind();
	
#ifdef TARGET_OPENGLES
	glDrawElements( GL_TRIANGLES, scratchIndices.size(), GL_UNSIGNED_SHORT, 	scratchIndices.data() );
#else
	glDrawElements( GL_TRIANGLES, scratchIndices.size(), GL_UNSIGNED_INT, 		scratchIndices.data() );
#endif
	
	unbind();
	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
}

//--------------------------------------------------------------
string ofxCubeMap::getDescriptiveStringForFace( GLuint _face )
{
	string tmpName = "";
	switch ( _face )
	{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			tmpName = "Pos X";
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			tmpName = "Neg X";
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			tmpName = "Pos Y";
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			tmpName = "Neg Y";
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			tmpName = "Pos Z";
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			tmpName = "Neg Z";
			break;
		default:
			ofLogError() << "ofxCubeMap::descriptiveStringForFace, passed in invalid face.";
			break;
    }
	
	return tmpName;
	
}

//--------------------------------------------------------------
void ofxCubeMap::setupSkyBoxVertices()
{
	float fExtent = 1.0f / 2.0f;
	
	///////////////////////////////////////////////
	//  Postive X
	cubemapTexCoords.push_back( ofVec3f(1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f(1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, fExtent) );
	
	
	//////////////////////////////////////////////
	// Negative X
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent , -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent , -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, -fExtent) );
		
	//////////////////////////////////////////////////
	// Positive Y
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, -fExtent) );
	
	///////////////////////////////////////////////////
	// Negative Y
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, fExtent) );
	
	
	////////////////////////////////////////////////
	// Positive Z
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, 1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, fExtent) );
	
	
	////////////////////////////////////////////////
	// Negative Z
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, -1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, -fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( -1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(-fExtent, fExtent, -fExtent) );
	
	cubemapTexCoords.push_back( ofVec3f( 1.0f, 1.0f, -1.0f) );
	cubemapVertices.push_back( ofVec3f(fExtent, fExtent, -fExtent) );
}

//--------------------------------------------------------------
void ofxCubeMap::initShader()
{
	
	// Begin GL3 version
	// We can't use '#' inside STRINGIFY
	string shaderVert = string("#version 330\nprecision highp float;\n") + STRINGIFY(
																					 
		 layout(location = 0) in vec4  position;
		 layout(location = 1) in vec4  color;
		 layout(location = 2) in vec3  normal;
		 layout(location = 3) in vec2  texcoord;
		 
		 uniform mat4 projectionMatrix;
		 uniform mat4 modelViewMatrix;
		 uniform mat4 modelViewProjectionMatrix;
		 uniform mat4 normalMatrix;
		 
		 out VertexAttrib {
			 vec3 texcoord;
		 } vertex;
		 
		 void main(void) 
		{
			vertex.texcoord = normalize(position.xyz);
			gl_Position = modelViewProjectionMatrix * position;
		}
	);
	// End GL3 version
	
	string shaderFrag = string("#version 330\nprecision highp float;\n") + STRINGIFY(
		 
		uniform samplerCube EnvMap;

		in VertexAttrib {
			vec3 texcoord;
		} vertex;

		out vec4 fragColor;

		void main (void)
		{
			vec4 envColor = texture(EnvMap, vertex.texcoord);
			fragColor = envColor;
		}
	);
	
	drawCubeMapShader.setupShaderFromSource(GL_VERTEX_SHADER, shaderVert );
	drawCubeMapShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderFrag );
	drawCubeMapShader.linkProgram();
	
}

