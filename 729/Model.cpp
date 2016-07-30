
#include "ModelCOM.h"

#include "mathheader.h"
#include "Render/IRender.h"
#include "xml/rapidxml_print.hpp"
#include "xml/UniConversion.h"
#include "xml/xfile.h"
#include "mLocal.h"
#include "xml/twxmlparser.h"
#include "DeleteList.h"
#include "CollisionData.h"
#include "Model.h"
#include"wcompress.h"
#include "common.h"

#include "utility/ModePack.h"

#include"zlib.h"

using namespace wnw;

Model::Model():
	m_bCompiled(false),
	m_bModified(false),
	m_strName(""),
	m_nUUID(0),
	m_pModelInfo(NULL),

#ifdef _MODEL_XFORM
		m_bIsLocal(false),
#endif
			m_nRefCount(0)

{


}
////////////////////////
Model::~Model()
{
	if(NULL !=m_pModelInfo)
	{
		delete m_pModelInfo;
		m_pModelInfo=NULL;
	  
	}

	iFree();
}
////////////////////
 bool   Model::iIsChanged()
 {
	 vector<IMesh*>::iterator it;
	 for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	 {
		 IMaterial*pMaterial = (*it)->iGetMaterial();
		 if( pMaterial )
		 {
			 if( pMaterial->iIsChanged() )
			 {
				 return true;
			 }
		 }
	 }
	 return false;
 }
////////
bool	Model::PackageModel(const char*strPath,const char*pModelName,ModelPrecision_e ePrecision)
{
	PackModel tPackModel;
	//////////
	char szBuff[64];
	strcpy(szBuff,pModelName);
	KConvert::To_LCase(szBuff);
	char*strExt = strstr(szBuff,".msh");
	if( strExt == NULL )
	{
		return false;
	}
	strcpy(strExt,".script");
	////////////
	string strPack = strPath;
	switch(ePrecision)
	{
	case MODEL_PREC_HIGH:
		{
			strPack[strPack.size()-1] = 'h';
		}
		break;
	case MODEL_PREC_MID:
		{
			strPack[strPack.size()-1] = 'm';
		}
		break;
	case MODEL_PREC_LOW:
		{
			strPack[strPack.size()-1] = 'l';
		}
		break;
	}

	tPackModel.SetPrecision(ePrecision);
	tPackModel.BeginWrite(strPack.c_str(),iVirtualFile);
	if(!tPackModel.WriteLump(pModelName,PackModel::MODEL) )
	{
		return false;
	}

	if(!tPackModel.WriteLump(szBuff,PackModel::SCRIPT,true) )
	{
		return false;
	}
	vector<IMesh*> ::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	{
		(*it)->iPackTexture(&tPackModel);
	}
	tPackModel.EndWrite();
	return true;

}
 /////
 bool	Model::iPackageModel(const char*strPath,const char*pModelName)
 {

	 if( !PackageModel(strPath,pModelName,MODEL_PREC_HIGH) )
	 {
		 return false;
	 }
	 if( !PackageModel(strPath,pModelName,MODEL_PREC_MID) )
	 {
		 return false;
	 }
	 if( !PackageModel(strPath,pModelName,MODEL_PREC_LOW) )
	 {
		 return false;
	 }
	return true;


 }
 ///////////

bool Model::LoadPackage_v43(void*pPackModel)
{
	
	PackModel *pPack = static_cast<PackModel*>(pPackModel);
	if( pPack == NULL )
	{
		return false;
	}
	TW43MeshHeader tModelHeader;
	byte* pScriptData  = NULL;
	int nScriptLen = 0;
	pPack->ReadScript(&pScriptData,nScriptLen);
	if(!LoadScript(pScriptData,nScriptLen,NULL,NULL, &tModelHeader,true))
	{
		delete [] pScriptData;
		return false;
	}
	delete [] pScriptData;
	int nLen, nPos;
	fileHandle_t hFile = pPack->GetFile();
	pPack->GetModelInfo(nLen,nPos);

	////////
	vector<TW43Lump> vecLump;
	if( !iVirtualFile->ReadFile(&tModelHeader,nPos,sizeof(TW43MeshHeader),hFile) )
	{
		return false;
	}
	ReadLump(hFile,nLen,vecLump,nPos);
	if( vecLump.size() < 0 )
	{
		return false;
	}
	m_nUUID = 0;
	m_tBBox.Clear();
	for(int i = 0; i < vecLump.size(); i ++)
	{
		if(vecLump[i].eType == LUMP_MESH )
		{
			IMesh *pMesh = iRender->iCreateMesh(MESH_WORLD);
			if( pMesh == NULL )
			{
				return  false;
			}
			TW43Lump &tMesh = vecLump[i];
			byte*pData = new byte[tMesh.nUnzipLen];
			//////////////////
			if( !iVirtualFile->ReadZipFile(pData,nPos+tMesh.GetDataPos(),tMesh.nZipLen,tMesh.nUnzipLen,hFile) )
			{
				delete [] pData;
				pData = NULL;
				return false;
			}	
			int nRet = pMesh->iLoad43(pData );
			if(  nRet > 0)
			{
				m_vecpMesh.push_back(pMesh);
				Vector3 v = pMesh->iGetBBox()._vMaxs;
				m_tBBox.Add(v);
				v = pMesh->iGetBBox()._vMins;
				m_tBBox.Add(v);
			}
			delete [] pData;
			pData = NULL;
		}
		else if(vecLump[i].eType == LUMP_COLLISION )
		{
			TW43Lump &tMesh = vecLump[i];
			byte*pData = new byte[tMesh.nUnzipLen];
			//////////////////
			if( !iVirtualFile->ReadZipFile(pData,tMesh.GetDataPos(),tMesh.nZipLen,tMesh.nUnzipLen,hFile) )
			{
				delete [] pData;
				pData = NULL;
				return false;
			}	
			if( m_pCollisionData == NULL )
			{
				m_pCollisionData = new CollisionData;
				m_pCollisionData->Load(pData,tMesh.nUnzipLen);
			}
			delete [] pData;
		}

	}
	////////
	//
	m_strName = tModelHeader.strName;
	vector<IMesh*>::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it)
	{
		LOGI("Load_v43 %s","iRegisterMaterial "); 
		(*it)->iRegisterMaterial(pPackModel);
	}
	LOGI("Load_v43 %s","end "); 
	return true;


}
/////////
void    Model::iFree()
{
///////////////////////////////////////////////////////////////
	vector<IMesh*> ::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	{
		iRender->iDestoryMesh(&(*it));
	}
	m_vecpMesh.clear();
}
///////////////////////////////////////////////
int Model::iGetTriangles()
{
	vector<IMesh*>::iterator it;
	int nCount = 0;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it)
	{
		nCount += (*it)->iGetElemCount()/3;
	}
	return nCount;
}
///////////////////////////////////////////////////////////////////////
 IMesh* Model::CreateMesh(MeshType_e eType)
 {
		return iRender->iCreateMesh(eType);
 }
///////////////////////////////////////////
 /////////////////////////////////
bool Model::LoadScript(byte* pScriptData,int nScriptLen,DeleteList*pDeleteList,XMLNodePtr pNode, TW43MeshHeader *pModelHeader,bool bUploadRender)
 {

	 ////////////////
		 if( nScriptLen >= 0 )
		 {
			 LoadModelInfo(pScriptData,nScriptLen);
			 if(bUploadRender)
			 {
				 iRender->iUploadRenderScript(pScriptData,nScriptLen);
			 }

		 }
			
	 return true;

 }

bool Model::Load_v43(fileHandle_t hFile,long lFileSize,DeleteList*pDeleteList,XMLNodePtr pNode)
{
	TW43MeshHeader tModelHeader;
	////////
	vector<TW43Lump> vecLump;
	byte* pScriptData  = NULL;
	int nScriptLen = 0;

	if( !iVirtualFile->ReadFile(&tModelHeader,sizeof(TW43MeshHeader),hFile) )
	{
		return false;
	}
	ReadLump(hFile,lFileSize,vecLump);
	if( vecLump.size() < 0 )
	{
		return false;
	}


	m_nUUID = 0;
	m_tBBox.Clear();
	for(int i = 0; i < vecLump.size(); i ++)
	{
		if(vecLump[i].eType == LUMP_MESH )
		{
			IMesh *pMesh = iRender->iCreateMesh(MESH_WORLD);
			if( pMesh == NULL )
			{
				return  false;
			}
			TW43Lump &tMesh = vecLump[i];
			byte*pData = new byte[tMesh.nUnzipLen];
			//////////////////
			if( !iVirtualFile->ReadZipFile(pData,tMesh.GetDataPos(),tMesh.nZipLen,tMesh.nUnzipLen,hFile) )
			{
				delete [] pData;
				pData = NULL;
				return false;
			}	
			int nRet = pMesh->iLoad43(pData );
			if(  nRet > 0)
			{
				m_vecpMesh.push_back(pMesh);
				Vector3 v = pMesh->iGetBBox()._vMaxs;
				m_tBBox.Add(v);
				v = pMesh->iGetBBox()._vMins;
				m_tBBox.Add(v);
			}
			delete [] pData;
		}
		else if(vecLump[i].eType == LUMP_COLLISION )
		{
			TW43Lump &tMesh = vecLump[i];
			byte*pData = new byte[tMesh.nUnzipLen];
			//////////////////
			if( !iVirtualFile->ReadZipFile(pData,tMesh.GetDataPos(),tMesh.nZipLen,tMesh.nUnzipLen,hFile) )
			{
				delete [] pData;
				pData = NULL;
				return false;
			}	
			if( m_pCollisionData == NULL )
			{
				m_pCollisionData = new CollisionData;
				m_pCollisionData->Load(pData,tMesh.nUnzipLen);
			}
			delete [] pData;
		}

	}
	////////
	//
	m_strName = tModelHeader.strName;
	vector<IMesh*>::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it)
	{
		LOGI("Load_v43 %s","iRegisterMaterial "); 
		(*it)->iRegisterMaterial(NULL);
	}
	LOGI("Load_v43 %s","end "); 
	return true;

}
/////////////////
///////////
/////////////////////////////
////////////////////////////////////////////////////////////////////////////
bool Model::MergeCompressModelFile(const char*pstrName,byte* pModelData,int nModelLen,byte* pScriptData,int nScriptLen)
{
	int nTotalLen = nModelLen+nScriptLen+4;
	byte*pData = new byte[nTotalLen];
	if(!pData)
	{
		return false;
	}
	memcpy(pData,pModelData,nModelLen);
	memcpy(pData+nModelLen,&nScriptLen,4);
	memcpy(pData+4+nModelLen,pScriptData,nScriptLen);

	WComprees comp;
	comp.CompressFile(pstrName,pData,nTotalLen);

	delete [] pData;
	pData=NULL;
	return true;
}
///////////
const BoundingBox&	Model::iGetBBox() 
{
		return m_tBBox;
}
IMesh **     Model::iGetRenderBuffer(int& nCount  )
{
		nCount = m_vecpMesh.size();
		return  &m_vecpMesh[0];
}

bool Model::iSetModelInfo(IModelInfo *pModelInfo)
{

	if(!m_pModelInfo)
	{
		IProtocol*iProtocol = (IProtocol*)iEngine->iFindInterface("iProtocol");	
		if(iProtocol==NULL)
		{
			return false;
		}
		iProtocol->New_Object(&m_pModelInfo);
		if(!m_pModelInfo)
		{
			return  false;
		}
	}

	if(!pModelInfo)
	{
		return false;
	}

	m_pModelInfo->SetID(pModelInfo->GetID());
	m_pModelInfo->SetType(pModelInfo->GetType());
	m_pModelInfo->SetClassID(pModelInfo->GetClassID());
	m_pModelInfo->SetGoodsID(pModelInfo->GetGoodsID());
	m_pModelInfo->SetStyleID(pModelInfo->GetStyleID());

	return true;

}

IModelInfo* Model::iGetModelInfo()
{
	return m_pModelInfo;
}



//////////////////////////////////////////////////////
bool	Model::iSaveModel(const char*strPath) 
{
	std::string _str = "model::iSaveModel";
	DEBUG_INFOR(_str.c_str());
	IProtocol*iProtocol = (IProtocol*)iEngine->iFindInterface("iProtocol");	
	assert(iProtocol);
	char           szLocalFile[MAX_PATH]={0};
	char strFile[MAX_PATH];

	if( strPath == NULL )
	{
		strcpy(strFile,m_strFileName.c_str());
	}
	else
	{
		strcpy(strFile,strPath);
	}

		iProtocol->GetSceneAbsolutePath(strFile,szLocalFile,sizeof(szLocalFile));
		///////////////
		xml_document<char> xmlDoc;

		xml_node<char>*pScript = xmlDoc.allocate_node(node_element,"SCRIPT");
		xmlDoc.append_node(pScript);

		if(NULL !=m_pModelInfo)
		{
			WriteModelInfo(xmlDoc,pScript);
		}

		for(int i = 0; i < m_vecpMesh.size(); i ++ )
		{
			m_vecpMesh[i]->iGetMaterialScript(xmlDoc);
		}
		////
		std::string text; 
		rapidxml::print(std::back_inserter(text), xmlDoc, 0); 
		int nUtf8Len = text.size()*3 + 1;
		char* u8 = new char[nUtf8Len];
		//	UTF8FromUCS2(text.c_str(),text.size(),u8,nUtf8Len);
		int  nScriptLen = ascii_to_utf8(text.c_str(),text.size(),u8);

		
		 KConvert::To_LCase(szLocalFile);

		 char*strExt = strstr(szLocalFile,".msh");
		 if( strExt == NULL )
		 {
			 return false;
		 }

		strcpy(strExt,".script");
		FILE*  pFile = fopen(szLocalFile,"w");
		if(NULL != pFile)
		{
			fwrite(u8,nScriptLen,1,pFile);
			fclose(pFile);
			DEBUG_INFOR( szLocalFile);
		}
		else
		{
			string strScript = "can't open the file:" + strScript;
			DEBUG_INFOR( strScript.c_str());
		}

//	else	if( nVersion == 42)
//	{
//	
//		byte *pCurData = pData;
//		int nVersion;
//		memcpy(&nVersion,pCurData,sizeof(int));
//
//		if( nVersion < 42)//
//		{
//			////////////////////////////
//			delete [] pData;
//			return false;
//
//		}
//		//////
//		TW42MeshHeader tModelHeader;
//		int nPos = 0;
//		memcpy(&tModelHeader,pData,sizeof(TW42MeshHeader));
//		nPos  += sizeof(TW42MeshHeader);
//		if( tModelHeader.nMeshType == MESH_COLLISION )
//		{
//			delete [] pData;
//			return false;
//		}
//		///////////////
//		xml_document<char> xmlDoc;
//
//		xml_node<char>*pScript = xmlDoc.allocate_node(node_element,"SCRIPT");
//		xmlDoc.append_node(pScript);
//
//		if(NULL !=m_pGoodsInfo)
//		{
//			WriteGoodsInfo(xmlDoc,pScript);
//		}
//
//		for(int i = 0; i < m_vecpMesh.size(); i ++ )
//		{
//			m_vecpMesh[i]->iGetMaterialScript(xmlDoc);
//		}
//		////
//		std::string text; 
//		rapidxml::print(std::back_inserter(text), xmlDoc, 0); 
//		int nUtf8Len = text.size()*3 + 1;
//		char* u8 = new char[nUtf8Len];
//		//	UTF8FromUCS2(text.c_str(),text.size(),u8,nUtf8Len);
//		int nScriptLen = ascii_to_utf8(text.c_str(),text.size(),u8);
//		char           szLocalFile[MAX_PATH]={0};
//		IProtocol*iProtocol = (IProtocol*)iEngine->iFindInterface("iProtocol");	
//		assert(iProtocol);
//		iProtocol->GetSceneAbsolutePath(m_strFileName.c_str(),szLocalFile,sizeof(szLocalFile));
//		MergeCompressModelFile(szLocalFile,pData,tModelHeader.nLen,(byte*)u8,nScriptLen);
//
//#if 1
//		string strScript = szLocalFile;
//		strScript +=".script";
//		FILE*  pFile = fopen(strScript.c_str(),"w");
//		if(NULL != pFile)
//		{
//			fwrite(u8,nScriptLen,1,pFile);
//			fclose(pFile);
//			DEBUG_INFOR( strScript.c_str());
//		}
//		else
//		{
//			strScript = "can't open the file:" + strScript;
//			DEBUG_INFOR( strScript.c_str());
//		}
//#endif
//		delete [] pData;
//		delete [] u8;
//		return true;
//
//	}

	return false;
}

const char*   Model::iGetFileName()  
{
	return m_strFileName.c_str();
}
void    Model::SetFileName(const char*szBuff) 
{
	m_strFileName = szBuff;
}
///////////////////////////////////////////////////////
bool    Model::iCompile() 
{
	if( m_bCompiled )
	{
		return true;
	}
	vector<IMesh*> ::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	{
		if ( !(*it)->iCompile() )
		{
			return false;
		}
	}
	m_bCompiled = true;
	return true;
}
//////
bool Model::iSetShadowParam(ShadowParam_t&tParam) 
{
	vector<IMesh*> ::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	{
			(*it)->iSetShadowParam(tParam);
	}
	m_bCompiled = false;
	return true;
}

//////
bool Model::iSetShadowParam(ShadowParam_t&tParam,vector<IMesh*>&vecpMesh) 
{
	vector<IMesh*> ::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it )
	{
		if( (*it)->iSetShadowParam(tParam) )
		{
			m_bCompiled = false;
			  for(int i = 0; i < vecpMesh.size(); i ++  )
			  {
				   if( *it == vecpMesh[i])
				   {
					    break;
				   }
			  }
		}
		vecpMesh.push_back(*it);
		}
	m_bCompiled = true;
	return false;
}
////////
#ifdef _MODEL_XFORM
void Model::iSetMatrix(const Matrix4& m,bool bSet )
{
		m_bIsLocal = bSet;
		if( m_bIsLocal )
		{
			 m_mModelMatrix = m;
		}

}
#endif
/////////////////
//////////
bool Model::Load_v42(byte*pData)
{
	TW42MeshHeader tModelHeader;
	int nPos = 0;
	memcpy(&tModelHeader,pData,sizeof(TW42MeshHeader));
	nPos  += sizeof(TW42MeshHeader);

	m_eMeshType = tModelHeader.nMeshType;
	if( m_eMeshType != MESH_COLLISION )
	{

		int nScriptLen;
		byte*pScriptData=NULL;

		pScriptData = pData+tModelHeader.nLen;
		memcpy(&nScriptLen,pScriptData,sizeof(int));
		if( nScriptLen >= 0 )
		{
			pScriptData += sizeof(int);
			LoadModelInfo(pScriptData,nScriptLen);

			iRender->iUploadRenderScript(pScriptData,nScriptLen);
		}
	}




	m_nUUID = 0;
	for(int  i = 0; i < tModelHeader.nNumMesh; i ++ )
	{
		IMesh *pMesh = iRender->iCreateMesh(m_eMeshType);
		if( pMesh == NULL )
		{
			return  false;
		}
		int nRet = pMesh->iLoad42(pData + nPos);
		if(  nRet > 0)
		{
			nPos += nRet;
			m_vecpMesh.push_back(pMesh);
			Vector3 v = pMesh->iGetBBox()._vMaxs;
			m_tBBox.Add(v);
			v = pMesh->iGetBBox()._vMins;
			m_tBBox.Add(v);

		}
		else 
		{
			iRender->iDestoryMesh(&pMesh);
		}


	}
	//m_strName = ws2s((wchar_t*)tModelHeader.strName).c_str();
	wstring wStrName = KConvert::To_wstring((unsigned short*)(tModelHeader.strName));
	m_strName =KConvert::To_astring(wStrName.c_str());
	if( tModelHeader.nMeshType != MESH_COLLISION )
	{

		vector<IMesh*>::iterator it;
		for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); ++ it)
		{
			LOGI("Load_v42 %s","iRegisterMaterial "); 
			(*it)->iRegisterMaterial(NULL);
		}
	}
	LOGI("Load_v42 %s","end "); 

	return true;

}


int Model::GetMeshlList(vector<IMesh*>& vecMesh)
{
	vector<IMesh*>::iterator it;
	for( it = m_vecpMesh.begin(); it != m_vecpMesh.end(); it ++ )
	{
		vecMesh.push_back(*it);
	}

	return STATUS_SUCCESS;
}



bool  Model::WriteModelInfo(xml_document<char> &xmlDoc,xml_node<char>*pRoot)
{
	char szTheID[100]={0};
	char*strValue =NULL;

	xml_node<char>*pNode =NULL;
	if(!m_pModelInfo)
	{
		return false;
		
	}

	xml_node<char>*pScript = xmlDoc.allocate_node(node_element,"MODELINFO");
	pRoot->append_node(pScript);

	sprintf(szTheID,"%d",m_pModelInfo->GetID());
	strValue = xmlDoc.allocate_string(szTheID);
	pNode= xmlDoc.allocate_node(node_element,"MODELID",strValue);
	pScript->append_node(pNode);

	strValue = xmlDoc.allocate_string(m_pModelInfo->GetUUID());
	pNode= xmlDoc.allocate_node(node_element,"UUID",strValue);
	pScript->append_node(pNode);



	sprintf(szTheID,"%d",m_pModelInfo->GetType());
	strValue = xmlDoc.allocate_string(szTheID);
	pNode= xmlDoc.allocate_node(node_element,"TYPEID",strValue);
	pScript->append_node(pNode);

	sprintf(szTheID,"%d",m_pModelInfo->GetClassID());
	strValue = xmlDoc.allocate_string(szTheID);
	pNode= xmlDoc.allocate_node(node_element,"CLASSID",strValue);
	pScript->append_node(pNode);

	sprintf(szTheID,"%d",m_pModelInfo->GetGoodsID());
	strValue = xmlDoc.allocate_string(szTheID);
	pNode= xmlDoc.allocate_node(node_element,"GOODSID",strValue);
	pScript->append_node(pNode);

	sprintf(szTheID,"%d",m_pModelInfo->GetStyleID());
	strValue = xmlDoc.allocate_string(szTheID);
	pNode= xmlDoc.allocate_node(node_element,"STYLEID",strValue);
	pScript->append_node(pNode);

	return true;
}

/////////
bool Model::ReadLump(fileHandle_t hFile,int nFileSize,vector<TW43Lump>&vecLump,int nOffset)
{
	int nPos = sizeof(TW43MeshHeader)+ nOffset;
	TW43Lump cLump;
	while(nPos < nFileSize )
	{

		if( !iVirtualFile->ReadFile(&cLump,nPos,sizeof(TW43Lump),hFile) )
		{
			return  false;
		}
		vecLump.push_back(cLump);
		nPos = nOffset + cLump.nPosition + cLump.nZipLen + sizeof(TW43Lump);

	}
	return true;

}
bool Model::LoadModelInfo(byte *pData,int nSize)
{
	int    nEncoding = -1;
	int    nTheID = 0;
	int    nLength=0;

	xFile file;
	bool bRet=true;

	XMLNodePtr  pNode =NULL;
	XMLNodePtr  pSubNode =NULL;
	xml_document<char>*pXml =NULL;
	const char *strTag="</MODELINFO>";

	char *pStart=NULL;
	char *pEnd=NULL;

	if((pData==NULL) || (nSize == 0))
	{
		return false;
	}

	pStart=strstr((char*)pData,"<MODELINFO>");
	if(!pStart)
	{
		return false;
	}

	pEnd =strstr(pStart,strTag);
	if(pEnd)
	{
		pEnd+=strlen(strTag);
		nLength = pEnd -pStart;
	}

    do
	{
		file.Input((const byte *)pStart, nLength,nEncoding);
		{
			pXml = new xml_document<char>;
			int Ls = file.GetBufferLength();
			char *pBuff = pXml->allocate_string(0,Ls);
			long nLen = file.ToASCII(pBuff);
			try
			{
				pXml->parse<0>(pBuff);
			}
			catch (parse_error& e)
			{
				wnw::Event tEvent(EVENT_G2A_MESSAGEBOX,(EVENT_PARAM)MB_OK,0,0);
				tEvent.SetStr("½âÎöXMLÎÄ¼þÊ§°Ü¡£");
				iEngine->iEventProc(tEvent);

				bRet=false;
				break;
			}


			pNode = pXml->first_node("MODELINFO");
			if( pNode  == NULL )
			{		
				bRet=false;
				break;
			}

			if(!m_pModelInfo)
			{
				IProtocol*iProtocol = (IProtocol*)iEngine->iFindInterface("iProtocol");	
				if(iProtocol==NULL)
				{
					bRet=false;
					break;
				}
				iProtocol->New_Object(&m_pModelInfo);
				if(!m_pModelInfo)
				{
					bRet=false;
					break;
				}
			}
			pSubNode= pNode->first_node("MODELID");
			if( pSubNode )
			{
				nTheID=atoi(pSubNode->value());
				m_pModelInfo->SetID(nTheID);
			}

			pSubNode= pNode->first_node("UUID");
			if( pSubNode )
			{
				m_pModelInfo->SetUUID(pSubNode->value());
			}

			pSubNode= pNode->first_node("TYPEID");
			if( pSubNode )
			{
				nTheID=atoi(pSubNode->value());
				m_pModelInfo->SetType((model_type_t)nTheID);
			}

			pSubNode= pNode->first_node("CLASSID");
			if( pSubNode )
			{
				nTheID=atoi(pSubNode->value());
				m_pModelInfo->SetClassID(nTheID);
			}

			pSubNode= pNode->first_node("GOODSID");
			if( pSubNode )
			{
				nTheID=atoi(pSubNode->value());
				m_pModelInfo->SetGoodsID(nTheID);
			}
			
			pSubNode= pNode->first_node("STYLEID");
			if( pSubNode )
			{
				nTheID=atoi(pSubNode->value());
				m_pModelInfo->SetStyleID(nTheID);
			}


		}

	}while(0);

	if(pXml  != NULL)
	{
		delete pXml;
		pXml = NULL;
	}

	return bRet;


}

//////////////