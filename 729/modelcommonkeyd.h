// ModelCommon.h
//
//////////////////////////////////////////////////////////////////////

#ifndef MODELCOMMON_H
#define MODELCOMMON_H
/////////////////////////////////////////////////////////

#pragma pack(1)

//////////////////////////////////////////////

#include "stdio.h"


#pragma  warning(disable:4786)
#pragma  warning(disable:4530)

#include <vector>


#define  CUR_VERSION 1.0
#define  MAX_SNAME   64
#define  MAX_SPATH   256

#define  MTL_CLASS_SHELL         "Shell Material"
#define  MTL_CLASS_MULTI_SUBOBJ  "Multi/Sub-Object"
#define  MTL_CLASS_STANDARD      "Standard" 
#define  MTL_CLASS_BAKED         "baked"

//#define  SHADER_NO
#define OBJ_CLASS_ALPHA         "alpha"

//Shader
#define  MAX_SHADER             8000


///
#define ATTRIB_NONE           0x0000

//World Enitity class
#define ATTRIB_WORLD          0x00000001
#define ATTRIB_COLLISION      0x00000002
#define ATTRIB_NO_COLLISION   0x00000004
#define ATTRIB_COLLISION_BOX  0x00000008


//Normal Entity class
#define ATTRIB_NORMAL         0x00000010

//Object Enitity class
#define ATTRIB_LIB            0x00000100
#define ATTRIB_TREE           0x00000200
#define ATTRIB_GRASS          0x00000400
#define ATTRIB_LOD_OBJECT     0x00000800
#define ATTRIB_BILLBOARD      0x00001000
//Animation
#define ATTRIB_ANIMATION       0x00010000

//Reflect water
#define ATTRIB_REFLECT_SURFACE  0x00100000

#define ATTRIB_SUB_OBJ			0x01000000
#define ATTRIB_INSTANCE			0x02000000
#define ATTRIB_LOD_LAYER	    0x04000000

//GROUP attribute
#define ATTRIB_GROUP_SUB_OPTION  0x10000000


typedef  char  String1024_t[1024];
typedef  char  String512_t[512];
typedef  char  String256_t[256];
typedef  char  String128_t[128];

typedef  char  String64_t[64];
typedef  char  String32_t[32];



using namespace std;
//////////////////////////////////////////////////////////////////////
#define MODEL_VERSION		2.0
namespace wnw
{


//////////////////////////////////////////////////////////////
typedef struct s_FileInfo
{
	int nVersion  ;
	int   nYear;
	int   nMonth;
	int   nDay;
}FileInfo_t;
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
typedef struct  s_FileHeader
{
	int			nVersion;
	int         nNumEntity;
	wnw::Vector3     vViewOrigin;
	wnw::Vector3     vViewAngles;
	int         nTotalFrame;
	float       fSpeed;
}FileHeader_t; 


class  TW4MeshHeader
{
public:
	int					nVersion;
	MeshType_e	        nMeshType;			       //Mesh类型
	unsigned __int64	nUUID;
	wnw::Vector3				vMins;
	wnw::Vector3				vMaxs;		       //包围盒
	int                 nNumMesh;
	char                strName[64];
private:
};

class  TW41MeshHeader
{
public:
	int					nVersion;
	MeshType_e	        nMeshType;			   //Mesh类型
	unsigned __int64	nUUID;
	wnw::Vector3				vMins;
	wnw::Vector3				vMaxs;		       //包围盒
	int                 nNumMesh;
	unsigned short      strName[64];
private:
};

class  TW42MeshHeader
{
public:
	int   nVersion;
	MeshType_e	        nMeshType;			       //Mesh类型
		wnw::Vector3				   vMins;
		wnw::Vector3		       vMaxs;		       //包围盒
	int                nNumMesh;
	short        strName[64];
	int            nLen;
	TW42MeshHeader():
	nVersion(0)
	{

	}
private:
};
//////
enum  LumpType_e
{
	LUMP_UNKOWN =-1,
	LUMP_MESH,
	LUMP_SCRIPT,
	LUMP_COLLISION,
};
//////////
class TW43Lump
{
public:
	LumpType_e eType;
	int  nPosition;
	int  nZipLen;
	int  nUnzipLen;
	TW43Lump&operator+(const TW43Lump&tmesh)
	{
		nPosition = tmesh.nPosition;
		nZipLen  = tmesh.nZipLen;
		nUnzipLen = tmesh.nUnzipLen;
        return *this;
	}
	TW43Lump():nPosition(0),
		nZipLen(0),
		nUnzipLen(0)
	{

	}
	int GetDataPos(){return (nPosition + sizeof(TW43Lump));}
};
class  TW43MeshHeader
{
public:
	short		nVersion;
	short       nLumpCount;
	char		strName[64];
	TW43MeshHeader():
	nVersion(43)
	{
	}
private:
};


}

#pragma pack()

//////////////////////////////////////////////////////////////////////

#endif