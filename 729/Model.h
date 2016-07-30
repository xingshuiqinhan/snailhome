#ifndef _DRAW_MDOEL_H_
#define _DRAW_MDOEL_H_
#include "utility/memFile.h"

#include "Render/IMesh.h"
#include "utility/Shared.h"


namespace wnw
{

class CollisionData;
class Model:public wnw::IModel
{

public:
	 Model();
	~Model();
	
	virtual const BoundingBox&  iGetBBox()  ;
	virtual IMesh **     iGetRenderBuffer(int& nCount);
	virtual bool	iSaveModel(const char*strPath = NULL) ;
	virtual bool   iSetModelInfo(IModelInfo *pModelInfo);
	virtual IModelInfo* iGetModelInfo();
	bool MergeCompressModelFile(const char*pstrName,byte* pModelData,int nModelLen,byte* pScriptData,int nScriptLen);
	virtual const char*   iGetFileName()  ;
	virtual void    SetFileName(const char*szBuff) ;
	virtual void    iFree();
	virtual int     iGetTriangles();
	virtual bool iSetShadowParam(ShadowParam_t&tParam) ;
	virtual bool iSetShadowParam(ShadowParam_t&tParam,vector<IMesh*>&vecpMesh) ;
	virtual bool    iIsChanged() ;
	virtual int    GetMeshlList(vector<IMesh*>& vecMesh);

	bool ReadLump(fileHandle_t hFile,int nFileSize,vector<TW43Lump>&vecLump,int nOffset = 0);
	virtual	 bool iCompile();
	 IMesh* CreateMesh(MeshType_e Type);
	 unsigned __int64 GetUUID() {return m_nUUID;}
	 bool Load_v42(byte*pData);
	 bool Load_v43(fileHandle_t hFile,long lFileSize,DeleteList*pDeleteList,XMLNodePtr pNode);
	 bool LoadPackage_v43(void*pPackModel);

	 //kangzh
	 bool LoadScript(byte* pScriptData,int nScriptLen,DeleteList*pDeleteList,XMLNodePtr pNode, TW43MeshHeader *pModelHeader,bool bUploadRender);//v43
	 bool LoadModelInfo(byte *pData,int nSize);
	 bool WriteModelInfo(xml_document<char> &xmlDoc,xml_node<char>*pRoot);

	 //////
	 bool	iPackageModel(const char*strPath,const char*pModelName); 


	  bool	PackageModel(const char*strPath,const char*pModelName,ModelPrecision_e ePrecision);
	 void AddRef()
	 {
		 m_nRefCount ++;
	 }
	 void ReleaseRef()
	 {
		 m_nRefCount --;
		 if( m_nRefCount == 0 )
		 {

		 }

	 }
private:
	IModelInfo  *m_pModelInfo;

	vector<IMesh*> m_vecpMesh;
	BoundingBox m_tBBox;
	unsigned __int64 m_nUUID;
	MeshType_e   m_eMeshType;
	string  m_strName;
	string  m_strFileName;

	int m_nRefCount;
	bool m_bCompiled;

	bool m_bModified;

	CollisionData*m_pCollisionData;
		////////

#ifdef _MODEL_XFORM
public:
		virtual void iSetMatrix(const Matrix4& m,bool bSet = false );
private:
		bool m_bIsLocal;
		Matrix4 m_mModelMatrix;
#endif
};

}
#endif