/*
  KeeFox - Allows Firefox to communicate with KeePass (via the KeeICE KeePass-plugin)
  Copyright 2008-2009 Chris Tomlinson <keefox@christomlinson.name>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// This is the implemenation of the XPCOM methods described in comp.idl

#include "comp-impl.h"

#if _MSC_VER
#pragma warning( push, 1 )
#pragma warning( disable : 4512 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4702 )
#endif

#include "nsICategoryManager.h"
#include "nsComponentManagerUtils.h"
#include "nsCOMArray.h"
#include "nsIObserverService.h"
#include "nsServiceManagerUtils.h"
#include "nsILocalFile.h"
#include "nsIProxyObjectManager.h"
//#include "nsStringAPI.h"

#include "shellapi.h"
#include "shlobj.h"

#include <string>
#include <vector>
#include <iostream>
//#include "nsILoginInfo.h"

#define IMPLEMENT_VISTA_TOOLS
#include "VistaTools.cxx"

#if _MSC_VER
#pragma warning( pop ) 
#endif

using namespace std;
using namespace KeeICE::KFlib;

using std::string;
using std::vector;

//NS_IMPL_ISUPPORTS1(CKeePass, IKeePass)
NS_IMPL_ADDREF(CKeeFox)
NS_IMPL_RELEASE(CKeeFox)

NS_INTERFACE_MAP_BEGIN(CKeeFox)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, CKeeFox)
  NS_INTERFACE_MAP_ENTRY(IKeeFox)
NS_INTERFACE_MAP_END

/*
NS_INTERFACE_MAP_BEGIN(CKeeFox)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, CKeeFox)
  NS_INTERFACE_MAP_ENTRY(nsILoginManagerStorage)
  NS_INTERFACE_MAP_ENTRY(IKeeFox)
NS_INTERFACE_MAP_END
*/

KeeFoxObserver *KeeFoxCallBackHelper::_observer = NULL;
bool KeeFoxCallBackHelper::javascriptCallBacksReady = false;


class CallbackReceiverI : public CallbackReceiver
{
public:

    virtual void
    callback(Ice::Int num, const Ice::Current&)
    {
		cout << "received callback ###" << num << endl;
		
		// send an asynchronous callback to the javascript observer. We don't
		// realy care if the observer has gone since there's nothing we can
		// do about that.
		//TODO: make this syncronous once .NET ensures only one call at a time (and from a thread that won't block further ICE calls to the .NET KeeICE server)
		if (KeeFoxCallBackHelper::javascriptCallBacksReady)
			KeeFoxCallBackHelper::_observer->CallBackToKeeFoxJS(num);

		// we want this to be the last callback made in some situations (i.e. KeePass closing)
		if (num == 12)
		{
			KeeFoxCallBackHelper::javascriptCallBacksReady = false;
//			KeeICE::KFlib->ShutdownICE();
		}
		//TODO: is there where we should release some XPCOM proxy memory?
		//KeeICE.ic.destroy();
    }
};

NS_IMETHODIMP CKeeFox::AddObserver(KeeFoxObserver *observer)
{
	nsresult rv = NS_OK;

	// if we've already set this up once for this firefox session just return
	//TODO: more thought needed here. will a ICE callback make it through to the 
	// javascript callback handler before the JS is ready for it? (in the case that 
	// KeeICE was started after KeeFox, or for a 2nd time...)
	if (KeeFoxCallBackHelper::javascriptCallBacksReady)
		return rv;

	// get the proxy manager object
	nsCOMPtr<nsIProxyObjectManager> pIProxyObjectManager =
	do_GetService("@mozilla.org/xpcomproxy;1", &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	// use proxy manager to create a proxy object for the observer we've been given
	// nsnull -> send proxied calls to the main Firefox UI thread
		//TODO: make this syncronous once .NET ensures only one call at a time (and from a thread that won't block further ICE calls to the .NET KeeICE server)
	rv = pIProxyObjectManager->GetProxyForObject( nsnull,
                                                  KeeFoxObserver::GetIID(),
                                                  observer,
                                                  0x0001 | 0x0004,
                                                  (void**)&KeeFoxCallBackHelper::_observer);
	// 0x0001 = synchronous, 0x0002 = async
	
	//TODO: these lines probably need to be uncommented to prevent memory leaks
    // we do not care about the real object anymore. ie. GetProxyObject
    // refcounts it.
	//NS_RELEASE(observer);
	//KeeFoxCallBackHelper::_observer->Test1(x,y,z);
	//NS_RELEASE(KeeFoxCallBackHelper::_observer);


	//KeeFoxCallBackHelper::_observer->SetJavascriptCallBacksReady((PRBool)true);
	KeeFoxCallBackHelper::javascriptCallBacksReady = true;


	// enum for status thoughts:
	//KF_STATUS_JSCALLBACKS_DISABLED 0
	//KF_STATUS_JSCALLBACKS_SETUP 1 // CALL THIS ONE FROM RIGHT HERE AS A QUICK SETUP SANITY TEST
	//KF_STATUS_ICECALLBACKS_SETUP 2
	//KF_STATUS_DATABASE_OPENING 3
	//KF_STATUS_DATABASE_OPEN 4
	//KF_STATUS_DATABASE_CLOSING 5
	//KF_STATUS_DATABASE_CLOSED 6
	//KF_STATUS_DATABASE_SAVING 7
	//KF_STATUS_DATABASE_SAVED 8
	//KF_STATUS_DATABASE_DELETING 9
	//KF_STATUS_DATABASE_DELETED 10
	//KF_STATUS_DATABASE_SELECTED 11
    //KF_STATUS_EXITING 12

	KeeFoxCallBackHelper::_observer->CallBackToKeeFoxJS(1);

	return 0;
}

CKeeFox::CKeeFox()
{
	/* member initializers and constructor code */
	//mName.Assign(L"Default Name");
	//int status = 0;
	//string DBName = "nope";

	
    //app.run(0, NULL);
//	char* argv[1];
//	argv[0] = "no";
//KeeICE.run(0, argv);
    
    
}

CKeeFox::~CKeeFox()
{
	/* destructor code */
	if (KeeICE.ic)
        KeeICE.ic->destroy();
}

int KeeICEProxy::run(int, char*[]) {
	return establishICEConnection();
}


int KeeICEProxy::establishICEConnection() {


        int status = 0;
		//string DBName = "nope";
		
		try {

			// Get the initialized property set.
			//
			Ice::PropertiesPtr props = Ice::createProperties();

			// Make sure that network and protocol tracing are off.
			//
			props->setProperty("Ice.ACM.Client", "0");
			props->setProperty("Ice.ThreadPool.Client.Size", "2");
			props->setProperty("Ice.ThreadPool.Server.Size", "2");

			//TODO: I suspect that 20 threads are not sufficient for normal operation. Looks like that's the side-effect of various bugs - hopefully squashed now but watch out when trying to drop this down again in v0.7.
			props->setProperty("Ice.ThreadPool.Client.SizeMax", "4");
			props->setProperty("Ice.ThreadPool.Server.SizeMax", "4");

			//props->setProperty("Ice.Trace.Protocol", "0");

			// Initialize a communicator with these properties.
			//
			Ice::InitializationData id;
			id.properties = props;
			ic = Ice::initialize(id);

			//ic = Ice::initialize();
			Ice::ObjectPrx base = ic->stringToProxy(
									"KeeICE:tcp -h localhost -p 12535");
			KP = KPPrx::checkedCast(base);
			if (!KP)
				throw "Invalid proxy";

			Ice::ObjectAdapterPtr adapter =	ic->createObjectAdapter("");
			Ice::Identity ident;
			ident.name = IceUtil::generateUUID();
			ident.category = "";
			//CallbackPtr cb = new CallbackI;
			CallbackReceiverPtr cb = new CallbackReceiverI;
			adapter->add(cb, ident);
			adapter->activate();
			KP->ice_getConnection()->setAdapter(adapter);
			KP->addClient(ident);

//    communicator()->waitForShutdown();

			;
		} catch (const Ice::Exception& ex) {
			cerr << ex << endl;
			status = 1;
			if (ic)
				ic->destroy();
		} catch (const char* msg) {
			cerr << msg << endl;
			status = 1;
			if (ic)
				ic->destroy();
		} catch (...) {
			//cerr << ex << endl;
			status = 1;
			if (ic)
				ic->destroy();
		}

        return status;
    }

		/*
int KeeICEProxy::main(int argc, char* argv[])
{
	int status = 0;
		//string DBName = "nope";
		
		try {

			// Get the initialized property set.
			//
			Ice::PropertiesPtr props = Ice::createProperties();

			// Make sure that network and protocol tracing are off.
			//
			props->setProperty("Ice.ACM.Client", "0");
			props->setProperty("Ice.ThreadPool.Client.Size", "2");
			props->setProperty("Ice.ThreadPool.Server.Size", "2");

			//TODO: I suspect that 20 threads are not sufficient for normal operation. Looks like that's the side-effect of various bugs - hopefully squashed now but watch out when trying to drop this down again in v0.7.
			props->setProperty("Ice.ThreadPool.Client.SizeMax", "200");
			props->setProperty("Ice.ThreadPool.Server.SizeMax", "200");

			//props->setProperty("Ice.Trace.Protocol", "0");

			// Initialize a communicator with these properties.
			//
			Ice::InitializationData id;
			id.properties = props;
			ic = Ice::initialize(id);

			//ic = Ice::initialize();
			Ice::ObjectPrx base = ic->stringToProxy(
									"KeeICE:tcp -h localhost -p 12535");
			KP = KPPrx::checkedCast(base);
			if (!KP)
				throw "Invalid proxy";

			Ice::ObjectAdapterPtr adapter =	ic->createObjectAdapter("");
			Ice::Identity ident;
			ident.name = IceUtil::generateUUID();
			ident.category = "";
			//CallbackPtr cb = new CallbackI;
			CallbackReceiverPtr cb = new CallbackReceiverI;
			adapter->add(cb, ident);
			adapter->activate();
			KP->ice_getConnection()->setAdapter(adapter);
			KP->addClient(ident);

//    communicator()->waitForShutdown();

			;
		} catch (const Ice::Exception& ex) {
			cerr << ex << endl;
			status = 1;
			if (ic)
				ic->destroy();
		} catch (const char* msg) {
			cerr << msg << endl;
			status = 1;
			if (ic)
				ic->destroy();
		} catch (...) {
			//cerr << ex << endl;
			status = 1;
			if (ic)
				ic->destroy();
		}

        return status;


*/







  /*  int status = EXIT_SUCCESS;
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc > 1)
        {
            fprintf(stderr, "%s: too many arguments\n", argv[0]);
            return EXIT_FAILURE;
        }
        HelloPrx hello = HelloPrx::checkedCast(communicator->stringToProxy("hello:tcp -p 10000"));
        if(!hello)
        {
            fprintf(stderr, "%s: invalid proxy\n", argv[0]);
            status = EXIT_FAILURE;
        }
        else
        {
            hello->sayHello();
        }
    }
    catch(const Ice::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;*/
//}

NS_IMETHODIMP CKeeFox::LaunchKeePass(nsAString const &fileName, nsAString const &DBFile)
{
	//TODO: this was an example of a random crash! nsString destructor called after already destroyed by something else? maybe another thread's GC? can we be sure it's the params below rather than input params above? if ones below then maybe some alternative way to convert the input vars in a way that we can destroy the objects before passing the native string to the execute method? if former, maybe it's becuase the javascript vars in calling function are defined as locals but that function shouldn't go out of scope before this function has completed...
ShellExecute(NULL,L"open",nsString(fileName).get(), nsString(DBFile).get(),L"",SW_SHOW );
return NS_OK;
}

//TODO: can we grab the return code to discover if the installation was successful?
NS_IMETHODIMP CKeeFox::RunAnInstaller(nsAString const &fileName, nsAString const &params)
{
	if (IsVista())
		RunElevated(NULL, nsString(fileName).get(), nsString(params).get(), NULL );
	else
		if (::IsUserAnAdmin())
			MyShellExec( NULL, L"open", nsString(fileName).get(), nsString(params).get(),L"", TRUE );
		else
			MyShellExec( NULL, L"runas", nsString(fileName).get(), nsString(params).get(),L"", TRUE );

	return NS_OK;
}

NS_IMETHODIMP CKeeFox::IsUserAdministrator(PRBool *_retval)
{
	if (IsVista())
	{
		

		TOKEN_ELEVATION_TYPE ptet;
		HRESULT res = GetElevationType(&ptet);
		if (res == S_OK && ptet != TokenElevationTypeDefault) 
		{
			// user has a split token so must be administrator
			*_retval = true;
		}
		else
		{
			// UAC is disabled or the user has limited rights, we'll try to find out the old fashioned way...
			*_retval = ::IsUserAnAdmin();
		}
	} else
	{
		*_retval = ::IsUserAnAdmin();
	}
	return NS_OK;
}



NS_IMETHODIMP CKeeFox::ShutdownICE()
{
	//TODO: what is going on here? should these three lines be uncommented to remove memory leaks? If so, need to stop everything deadlocking when they execute!
	//KeeICE.ic->shutdown();
	//KeeICE.ic->waitForShutdown(); // destroy? // or assign to null? // look up about killing dead proxies?
	//KeeICE.ic->destroy();
	KeeICE.KP = NULL;
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetDBName(nsAString &_retval)
{
	int status = 0;
	string DBName = "no name";

    try {
		if (!KeeICE.KP)
            status = 1;
		else
			DBName = KeeICE.KP->getDatabaseName();
    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

	if (status)
		return NS_ERROR_NOT_AVAILABLE;

	// Convert response to XPCOM string
	_retval = NS_ConvertUTF8toUTF16(DBName.c_str());
	
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetDBFileName(nsAString &_retval)
{
	int status = 0;
	string DBFileName = "nope";

    try {
		if (!KeeICE.KP)
            status = 1;
		else
			DBFileName = KeeICE.KP->getDatabaseFileName();
    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

	if (status)
		return NS_ERROR_NOT_AVAILABLE;

	// Convert response to XPCOM string
	_retval = NS_ConvertUTF8toUTF16(DBFileName.c_str());
	
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::ChangeDB(nsAString const &fileName, PRBool closeCurrent)
{
	int status = 0;
	//string DBFileName = "nope";

    try {
		if (!KeeICE.KP)
            throw "Proxy went away";

		KeeICE.KP->changeDatabase(NS_ConvertUTF16toUTF8(fileName).get(), closeCurrent != 0);
        ;
    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

	// Convert response to XPCOM string
	//_retval = NS_ConvertUTF8toUTF16(DBFileName.c_str());
	
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::CheckVersion(float keeFoxVersion, float keeICEVersion, PRInt32 *result, PRBool *_retval)
{
	int status = 0;
	*_retval = false;

    try {
		if (!KeeICE.KP)
		{
			status = KeeICE.establishICEConnection();
		}
		//throw "Proxy went away";
		if (!status)
			*_retval = KeeICE.KP->checkVersion(keeFoxVersion, keeICEVersion, *result);
    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

	return NS_OK;
}

/* attribute AString name; */
NS_IMETHODIMP CKeeFox::GetName(nsAString & aName)
{
	aName.Assign(mName);
	return NS_OK;
}
NS_IMETHODIMP CKeeFox::SetName(const nsAString & aName)
{
	mName.Assign(aName);
	return NS_OK;
}





KeeICE::KFlib::KPEntry CKeeFox::ConvertLoginInfoToKPEntry (kfILoginInfo *aLogin)
{
	KeeICE::KFlib::KPEntry entry;
	KPFormFieldList *formFieldList = new KPFormFieldList();
	entry.formFieldList = *formFieldList;

	nsString URL;
	(void)aLogin->GetURL(URL);
	entry.URL = NS_ConvertUTF16toUTF8(URL).get(); 
	
	//entry.title = entry.hostName;

	nsString formActionURL;
	(void)aLogin->GetFormActionURL(formActionURL);
	entry.formActionURL = NS_ConvertUTF16toUTF8(formActionURL).get();

	nsString httpRealm;
	(void)aLogin->GetHttpRealm(httpRealm);
	entry.HTTPRealm = NS_ConvertUTF16toUTF8(httpRealm).get(); 

	nsString uniqueID;
	(void)aLogin->GetUniqueID(uniqueID);
	entry.uniqueID = NS_ConvertUTF16toUTF8(uniqueID).get();

	nsString title;
	(void)aLogin->GetTitle(title);
	entry.title = NS_ConvertUTF16toUTF8(title).get();

	nsString username;
	(void)aLogin->GetUsername(username);
	nsString usernameField;
	(void)aLogin->GetUsernameField(usernameField);

	KPFormField *usernameFF = new KPFormField();
	usernameFF->name = NS_ConvertUTF16toUTF8(usernameField).get(); 
	usernameFF->value = NS_ConvertUTF16toUTF8(username).get();
	usernameFF->type = formFieldType::FFTusername;

	entry.formFieldList.push_back(*usernameFF);

	nsString password;
	(void)aLogin->GetPassword(password);
	nsString passwordField;
	(void)aLogin->GetPasswordField(passwordField);

	KPFormField *passwordFF = new KPFormField();
	passwordFF->name = NS_ConvertUTF16toUTF8(passwordField).get(); 
	passwordFF->value = NS_ConvertUTF16toUTF8(password).get();
	passwordFF->type = formFieldType::FFTpassword;

	entry.formFieldList.push_back(*passwordFF);

	nsCOMPtr<nsIMutableArray> customFields;
	(void)aLogin->GetCustomFields(getter_AddRefs(customFields));

	// get the array
	//nsCOMPtr<nsIArray> array;
	//foo->GetElements(getter_AddRefs(array));
	 

	if (customFields != NULL)
	{
		PRUint32 customCount = 0;
		customFields->GetLength(&customCount);

		if (customCount > 0)
		{
			// make an enumerator
			nsCOMPtr<nsISimpleEnumerator> enumerator;
			customFields->Enumerate(getter_AddRefs(enumerator));

			// now enumerate the elements
			PRBool moreFields;
			enumerator->HasMoreElements(&moreFields);


			while (moreFields)
			{
				nsCOMPtr<kfILoginField> customField;
				enumerator->GetNext(getter_AddRefs(customField));

				nsString fieldValue;
				(void)customField->GetValue(fieldValue);
				nsString fieldName;
				(void)customField->GetName(fieldName);

				KPFormField *customFF = new KPFormField();
				customFF->name = NS_ConvertUTF16toUTF8(fieldName).get(); 
				customFF->value = NS_ConvertUTF16toUTF8(fieldValue).get();
				customFF->type = formFieldType::FFTtext; //TODO: maybe want to record an accurate type here one day?

				entry.formFieldList.push_back(*customFF);

				enumerator->HasMoreElements(&moreFields);
			}
		}
	}
	return entry;
}



nsCOMPtr<kfILoginInfo> CKeeFox::ConvertKPEntryToLoginInfo (KeeICE::KFlib::KPEntry entry)
{
	nsresult rv;
	nsString username, password, usernameField, passwordField;

		nsCOMPtr<nsIMutableArray> customFields = do_CreateInstance(NS_ARRAY_CONTRACTID);
				
		for (unsigned int j = 0; j < entry.formFieldList.size(); j++) 
		{
			KPFormField kpff = entry.formFieldList.at(j);
	#if _DEBUG
	  std::cout << "found a new field..." << "\n";
	#endif
	  if (kpff.type == formFieldType::FFTusername)
			{
				username = NS_ConvertUTF8toUTF16(kpff.value.c_str());
				usernameField = NS_ConvertUTF8toUTF16(kpff.name.c_str());
	#if _DEBUG
	  std::cout << "found a username" << "\n";
	#endif
			} else if (kpff.type == formFieldType::FFTpassword)
			{
				password = NS_ConvertUTF8toUTF16(kpff.value.c_str());
				passwordField = NS_ConvertUTF8toUTF16(kpff.name.c_str());
	#if _DEBUG
	  std::cout << "found a password" << "\n";
	#endif
			} else if (kpff.type == formFieldType::FFTtext)
			{
				nsString customValue = NS_ConvertUTF8toUTF16(kpff.value.c_str());
				nsString customName = NS_ConvertUTF8toUTF16(kpff.name.c_str());

				
				nsCOMPtr<kfILoginField> customField = do_CreateInstance("@christomlinson.name/kfLoginField;1");
				if (!customField) {
				  return NULL;
				}

				rv = customField->Init(customName, customValue);

				if (NS_SUCCEEDED(rv))
					customFields->AppendElement(customField,false);


	#if _DEBUG
	  std::cout << "found a text/custom field" << "\n";
	#endif
			} else
			{
	#if _DEBUG
	  std::cout << "not a username, password or text/custom field - unsupported." << "\n";
	#endif
			}
		}

		nsString NSURL,NSactionURL,NSHTTPRealm,NSuniqueID,NStitle;

		NSURL = NS_ConvertUTF8toUTF16(entry.URL.c_str());
		NSactionURL = NS_ConvertUTF8toUTF16(entry.formActionURL.c_str());
		NSHTTPRealm = NS_ConvertUTF8toUTF16(entry.HTTPRealm.c_str());
		NSuniqueID = NS_ConvertUTF8toUTF16(entry.uniqueID.c_str());
		NStitle = NS_ConvertUTF8toUTF16(entry.title.c_str());

		nsCOMPtr<kfILoginInfo> info = do_CreateInstance("@christomlinson.name/kfLoginInfo;1");
		if (!info) {
		  return NULL;
		}

		PRUint32 customLength = 0;

		customFields->GetLength(&customLength);

		if (customLength > 0)
			rv = info->InitCustom(NSURL, NSactionURL, NSHTTPRealm, username, password,
						usernameField, passwordField, NSuniqueID, NStitle, customFields);
		else
			rv = info->Init(NSURL, NSactionURL, NSHTTPRealm, username, password,
						usernameField, passwordField, NSuniqueID, NStitle);

		if (NS_SUCCEEDED(rv))
		  return info;
}




KeeICE::KFlib::KPGroup CKeeFox::ConvertGroupInfoToKPGroup (kfIGroupInfo *aGroup)
{
	KeeICE::KFlib::KPGroup group;
	
	nsString name;
	(void)aGroup->GetTitle(name);
	group.title = NS_ConvertUTF16toUTF8(name).get(); 

	nsString uniqueID;
	(void)aGroup->GetUniqueID(uniqueID);
	group.uniqueID = NS_ConvertUTF16toUTF8(uniqueID).get();

	return group;
}



nsCOMPtr<kfIGroupInfo> CKeeFox::ConvertKPGroupToGroupInfo (KeeICE::KFlib::KPGroup group)
{
	nsresult rv;
	nsString name, uniqueRef;
	nsString NSname, NSuniqueID;

	NSname = NS_ConvertUTF8toUTF16(group.title.c_str());
	NSuniqueID = NS_ConvertUTF8toUTF16(group.uniqueID.c_str());

	nsCOMPtr<kfIGroupInfo> info = do_CreateInstance("@christomlinson.name/kfGroupInfo;1");
	if (!info) {
	  return NULL;
	}

	rv = info->Init(NSname, NSuniqueID);

	if (NS_SUCCEEDED(rv))
	  return info;
}





NS_IMETHODIMP CKeeFox::AddLogin(kfILoginInfo *aLogin, const nsAString &parentUUID, kfILoginInfo **aNewLogin)
{
  NS_ENSURE_ARG_POINTER(aLogin);

#if _DEBUG
  std::cout << "comp-impl.cpp::AddLogin - start" << "\n";
#endif

KeeICE::KFlib::KPEntry entry;
entry = ConvertLoginInfoToKPEntry(aLogin);

  try {
	if (!KeeICE.KP)
        throw "Proxy went away";

		entry = KeeICE.KP->AddLogin(entry, NS_ConvertUTF16toUTF8(parentUUID).get());

		// does this work with mutable array? mem overflow?
	  kfILoginInfo *retval = (kfILoginInfo *)NS_Alloc(sizeof(kfILoginInfo *));
	  //for (PRInt32 i = 0; i < results.Count(); i++)
		NS_ADDREF(retval = ConvertKPEntryToLoginInfo(entry));
	  *aNewLogin = retval;

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
    } catch (const char* msg) {
        cerr << msg << endl;
	} catch (...) {
    }

#if _DEBUG
  std::cout << "comp-impl.cpp::AddLogin - finished" << "\n";
#endif

  return NS_OK;
}


NS_IMETHODIMP CKeeFox::DeleteLogin(const nsAString &uniqueID, PRBool *_retval)
{
	*_retval = KeeICE.KP->removeEntry(NS_ConvertUTF16toUTF8(uniqueID).get());
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::ModifyLogin(kfILoginInfo *aOldLogin, kfILoginInfo *aNewLogin)
{
  NS_ENSURE_ARG_POINTER(aOldLogin);
  NS_ENSURE_ARG_POINTER(aNewLogin);

#if _DEBUG
  std::cout << "comp-impl.cpp::ModifyLogin - start" << "\n";
#endif

KeeICE::KFlib::KPEntry oldEntry;
oldEntry = ConvertLoginInfoToKPEntry(aOldLogin);
KeeICE::KFlib::KPEntry newEntry;
newEntry = ConvertLoginInfoToKPEntry(aNewLogin);

  try {
	if (!KeeICE.KP)
        throw "Proxy went away";

		KeeICE.KP->ModifyLogin(oldEntry, newEntry);

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
    } catch (const char* msg) {
        cerr << msg << endl;
	} catch (...) {
    }

#if _DEBUG
  std::cout << "comp-impl.cpp::ModifyLogin - finished" << "\n";
#endif

  return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetAllLogins(PRUint32 *count, kfILoginInfo ***logins)
{

#if _DEBUG
  std::cout << "comp-impl.cpp::GetAllLogins - started" << "\n";
#endif

  int status = 0;
  nsresult rv;

  nsCOMArray<kfILoginInfo> results;

  KeeICE::KFlib::KPEntryList entries;

  try {
		if (!KeeICE.KP)
            throw "Proxy went away";

		*count = KeeICE.KP->getAllLogins(entries);

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

#if _DEBUG
  std::cout << "we have received " << entries.size() << " KPEntry objects" << "\n";
#endif

	for (unsigned int i = 0; i < entries.size(); i++) {
		KPEntry entry = entries.at(i);

	#if _DEBUG
	  std::cout << "found a new KPEntry" << "\n";
	#endif

	  nsCOMPtr<kfILoginInfo> info;

		info = ConvertKPEntryToLoginInfo(entry);
		if (info)
			(void)results.AppendObject(info);

	#if _DEBUG
	  std::cout << "appended new kfILoginInfo object" << "\n";
	#endif
	}

  if (0 == results.Count()) {
    *logins = nsnull;
    *count = 0;
    return NS_OK;
  }
  
  kfILoginInfo **retval = (kfILoginInfo **)NS_Alloc(sizeof(kfILoginInfo *) * results.Count());
  for (PRInt32 i = 0; i < results.Count(); i++)
    NS_ADDREF(retval[i] = results[i]);
  *logins = retval;
  *count = results.Count();
  
#if _DEBUG
  std::cout << "count: " << *count << "\n";
#endif

#if _DEBUG
  std::cout << "comp-impl.cpp::GetAllLogins - finished" << "\n";
#endif

  return NS_OK;

}

NS_IMETHODIMP CKeeFox::FindLogins(PRUint32 *count, const nsAString &aHostname, const nsAString &aActionURL, const nsAString &aHttpRealm, const nsAString &aUniqueID, kfILoginInfo ***logins)
{
 
#if _DEBUG
  std::cout << "comp-impl.cpp::FindLogins - started" << "\n";
#endif

  int status = 0;
  

  nsCOMArray<kfILoginInfo> results;

  KeeICE::KFlib::KPEntryList entries;

  try {
		if (!KeeICE.KP)
            throw "Proxy went away";

		// null (Void) strings and empty strings are different in Firefox but ICE can't support that so there are
		// various places like this where we need to make some decisions about what KeePass can return before
		// any message is actually sent to KeePass. A bit counter-intuitive but it works and reduces
		// inter-application traffic too as a nice side-effect.
		if (aActionURL.IsVoid() && aHttpRealm.IsVoid())
		{	
			*count = 0;
			#if _DEBUG
			  std::cout << "comp-impl.cpp::FindLogins - finished" << "\n";
			#endif
			return NS_OK;
		}

		string hostname = NS_ConvertUTF16toUTF8(aHostname).get();
		string actionURL = NS_ConvertUTF16toUTF8(aActionURL).get();
		string httpRealm = NS_ConvertUTF16toUTF8(aHttpRealm).get();
		string uniqueID = NS_ConvertUTF16toUTF8(aUniqueID).get();

		

		if (aHttpRealm.IsVoid())
			*count = KeeICE.KP->findLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTnoRealms),false, uniqueID, entries);
		else if (aActionURL.IsVoid())
			*count = KeeICE.KP->findLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTnoForms),false, uniqueID, entries);
		else
			*count = KeeICE.KP->findLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTall), false, uniqueID, entries);

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

#if _DEBUG
  std::cout << "we have received " << entries.size() << " KPEntry objects" << "\n";
#endif

	for (unsigned int i = 0; i < entries.size(); i++) {
		KPEntry entry = entries.at(i);

	#if _DEBUG
	  std::cout << "found a new KPEntry" << "\n";
	#endif

	  nsCOMPtr<kfILoginInfo> info;

		info = ConvertKPEntryToLoginInfo(entry);
		if (info)
			(void)results.AppendObject(info);

	#if _DEBUG
	  std::cout << "appended new kfILoginInfo object" << "\n";
	#endif
	}

  if (0 == results.Count()) {
    *logins = nsnull;
    *count = 0;
#if _DEBUG
	std::cout << "count: " << *count << "\n";
	std::cout << "comp-impl.cpp::FindLogins - finished" << "\n";
#endif
    return NS_OK;
  }
  
  // does this work with mutable array? mem overflow?
  kfILoginInfo **retval = (kfILoginInfo **)NS_Alloc(sizeof(kfILoginInfo *) * results.Count());
  for (PRInt32 i = 0; i < results.Count(); i++)
    NS_ADDREF(retval[i] = results[i]);
  *logins = retval;
  *count = results.Count();
  
#if _DEBUG
  std::cout << "count: " << *count << "\n";
#endif

#if _DEBUG
  std::cout << "comp-impl.cpp::FindLogins - finished" << "\n";
#endif

  return NS_OK;
}

NS_IMETHODIMP CKeeFox::CountLogins(nsAString const &aHostname, nsAString const &aActionURL,
	nsAString const &aHttpRealm, unsigned int *_retval)
{
#if _DEBUG
  std::cout << "comp-impl.cpp::CountLogins - started" << "\n";
#endif

	int status = 0;
	int count = 0;

    try {
		if (!KeeICE.KP)
            throw "Proxy went away";

		// null (Void) strings and empty strings are different in Firefox but ICE can't support that concept
		// so there are various places like this where we need to make some decisions about what KeePass
		// can return before any message is actually sent to KeePass. A bit counter-intuitive but
		// it works and reduces inter-application traffic too as a nice side-effect.
		if (aActionURL.IsVoid() && aHttpRealm.IsVoid())
		{	
			*_retval = 0;
			#if _DEBUG
			  std::cout << "comp-impl.cpp::CountLogins - finished" << "\n";
			#endif
			return NS_OK;
		}

		string hostname = NS_ConvertUTF16toUTF8(aHostname).get();
		string actionURL = NS_ConvertUTF16toUTF8(aActionURL).get();
		string httpRealm = NS_ConvertUTF16toUTF8(aHttpRealm).get();

		if (aHttpRealm.IsVoid())
			*_retval = KeeICE.KP->countLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTnoRealms), false);
		else if (aActionURL.IsVoid())
			*_retval = KeeICE.KP->countLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTnoForms), false);
		else
			*_retval = KeeICE.KP->countLogins(hostname,actionURL,httpRealm,KeeICE::KFlib::loginSearchType(LSTall), false);

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

#if _DEBUG
  std::cout << "comp-impl.cpp::CountLogins - finished" << "\n";
#endif
//*_retval = 1;
return NS_OK;
}




NS_IMETHODIMP CKeeFox::FindGroups(PRUint32 *count, const nsAString &aName, const nsAString &aUniqueID, kfIGroupInfo ***logins)
{
 
#if _DEBUG
  std::cout << "comp-impl.cpp::FindGroups - started" << "\n";
#endif

  int status = 0;
  
  nsCOMArray<kfIGroupInfo> results;

  KeeICE::KFlib::KPGroupList groups;

  try {
		if (!KeeICE.KP)
            throw "Proxy went away";

		string name = NS_ConvertUTF16toUTF8(aName).get();
		string uniqueID = NS_ConvertUTF16toUTF8(aUniqueID).get();

		*count = KeeICE.KP->findGroups(name, uniqueID, groups);

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
		status = 1;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
	} catch (...) {
        status = 1;
    }

#if _DEBUG
  std::cout << "we have received " << groups.size() << " KPGroup objects" << "\n";
#endif

	for (unsigned int i = 0; i < groups.size(); i++) {
		KPGroup group = groups.at(i);

	#if _DEBUG
	  std::cout << "found a new KPGroup" << "\n";
	#endif

	  nsCOMPtr<kfIGroupInfo> info;

		info = ConvertKPGroupToGroupInfo(group);
		if (info)
			(void)results.AppendObject(info);

	#if _DEBUG
	  std::cout << "appended new kfIGroupInfo object" << "\n";
	#endif
	}

  if (0 == results.Count()) {
    *logins = nsnull;
    *count = 0;
#if _DEBUG
	std::cout << "count: " << *count << "\n";
	std::cout << "comp-impl.cpp::FindGroups - finished" << "\n";
#endif
    return NS_OK;
  }
  
  // does this work with mutable array? mem overflow?
  kfIGroupInfo **retval = (kfIGroupInfo **)NS_Alloc(sizeof(kfIGroupInfo *) * results.Count());
  for (PRInt32 i = 0; i < results.Count(); i++)
    NS_ADDREF(retval[i] = results[i]);
  *logins = retval;
  *count = results.Count();
  
#if _DEBUG
  std::cout << "count: " << *count << "\n";
#endif

#if _DEBUG
  std::cout << "comp-impl.cpp::FindGroups - finished" << "\n";
#endif

  return NS_OK;
}



NS_IMETHODIMP CKeeFox::AddGroup(const nsAString &name, const nsAString &parentUUID, kfIGroupInfo **aGroup)
{

#if _DEBUG
  std::cout << "comp-impl.cpp::AddGroup - start" << "\n";
#endif

KeeICE::KFlib::KPGroup group;

  try {
	if (!KeeICE.KP)
        throw "Proxy went away";

		group = KeeICE.KP->addGroup(NS_ConvertUTF16toUTF8(name).get(), NS_ConvertUTF16toUTF8(parentUUID).get());

		// does this work with mutable array? mem overflow?
		kfIGroupInfo *retval = (kfIGroupInfo *)NS_Alloc(sizeof(kfIGroupInfo *));
		//for (PRInt32 i = 0; i < results.Count(); i++)
		NS_ADDREF(retval = ConvertKPGroupToGroupInfo(group));
		*aGroup = retval;

    } catch (KeeICE::KFlib::KeeICEException ex) {
		cerr << ex.reason << endl;
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
    } catch (const char* msg) {
        cerr << msg << endl;
	} catch (...) {
    }

#if _DEBUG
  std::cout << "comp-impl.cpp::AddGroup - finished" << "\n";
#endif

  return NS_OK;
}

NS_IMETHODIMP CKeeFox::DeleteGroup(const nsAString &uniqueID, PRBool *_retval)
{
	*_retval = KeeICE.KP->removeGroup(NS_ConvertUTF16toUTF8(uniqueID).get());
	return NS_OK;
}


NS_IMETHODIMP CKeeFox::GetParentGroup(const nsAString &uniqueID, kfIGroupInfo **aGroup)
{
	KeeICE::KFlib::KPGroup group;
	group = KeeICE.KP->getParent(NS_ConvertUTF16toUTF8(uniqueID).get());
	
	kfIGroupInfo *retval = (kfIGroupInfo *)NS_Alloc(sizeof(kfIGroupInfo *));
    NS_ADDREF(retval = ConvertKPGroupToGroupInfo(group));
  *aGroup = retval;

	return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetRootGroup(kfIGroupInfo **aGroup)
{
	//std::cout << "comp-impl.cpp::GetRootGroup - started" << "\n";

	KeeICE::KFlib::KPGroup group;

	try {
		group = KeeICE.KP->getRoot(); // is group going out of scope / being GCd?
			;
		} catch (const Ice::Exception& ex) {
			cout << ex << endl;
		} catch (const char* msg) {
			cout << msg << endl;
		} catch (...) {
			cout << "getRoot exception :-(" << endl;
		}

	kfIGroupInfo *retval = (kfIGroupInfo *)NS_Alloc(sizeof(kfIGroupInfo *));
    NS_ADDREF(retval = ConvertKPGroupToGroupInfo(group));
  *aGroup = retval;

  //std::cout << "comp-impl.cpp::GetRootGroup - finished" << "\n";

	return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetChildGroups(PRUint32 *count, const nsAString &uniqueID, kfIGroupInfo ***nsGroups)
{
#if _DEBUG
  std::cout << "comp-impl.cpp::GetChildGroups - started" << "\n";
#endif

	nsCOMArray<kfIGroupInfo> results;

	KeeICE::KFlib::KPGroupList groups;

	groups = KeeICE.KP->getChildGroups(NS_ConvertUTF16toUTF8(uniqueID).get());

	for (unsigned int i = 0; i < groups.size(); i++) {
		KPGroup group = groups.at(i);

	#if _DEBUG
	  std::cout << "found a new KPGroup" << "\n";
	#endif

	  nsCOMPtr<kfIGroupInfo> info;

		info = ConvertKPGroupToGroupInfo(group);
		if (info)
			(void)results.AppendObject(info);

	#if _DEBUG
	  std::cout << "appended new kfIGroupInfo object" << "\n";
	#endif
	}

	kfIGroupInfo **retval = (kfIGroupInfo **)NS_Alloc(sizeof(kfIGroupInfo *) * results.Count());
	for (PRInt32 i = 0; i < results.Count(); i++)
	NS_ADDREF(retval[i] = results[i]);
	*nsGroups = retval;
	*count = results.Count();
	  
	#if _DEBUG
	  std::cout << "count: " << *count << "\n";
	#endif

	#if _DEBUG
	  std::cout << "comp-impl.cpp::GetChildGroups - finished" << "\n";
	#endif

	return NS_OK;
}

NS_IMETHODIMP CKeeFox::GetChildEntries(PRUint32 *count, const nsAString &uniqueID, kfILoginInfo ***nsLogins)
{
#if _DEBUG
  std::cout << "comp-impl.cpp::GetChildEntries - started" << "\n";
#endif

	nsCOMArray<kfILoginInfo> results;

	KeeICE::KFlib::KPEntryList logins;

	logins = KeeICE.KP->getChildEntries(NS_ConvertUTF16toUTF8(uniqueID).get());

	for (unsigned int i = 0; i < logins.size(); i++) {
		KPEntry login = logins.at(i);

	#if _DEBUG
	  std::cout << "found a new KPEntry" << "\n";
	#endif

	  nsCOMPtr<kfILoginInfo> info;

		info = ConvertKPEntryToLoginInfo(login);
		if (info)
			(void)results.AppendObject(info);

	#if _DEBUG
	  std::cout << "appended new kfILoginInfo object" << "\n";
	#endif
	}

	kfILoginInfo **retval = (kfILoginInfo **)NS_Alloc(sizeof(kfILoginInfo *) * results.Count());
	for (PRInt32 i = 0; i < results.Count(); i++)
	NS_ADDREF(retval[i] = results[i]);
	*nsLogins = retval;
	*count = results.Count();
	  
	#if _DEBUG
	  std::cout << "count: " << *count << "\n";
	#endif

	#if _DEBUG
	  std::cout << "comp-impl.cpp::GetChildEntries - finished" << "\n";
	#endif

	return NS_OK;
}

NS_IMETHODIMP CKeeFox::LaunchGroupEditor(const nsAString &uniqueID)
{
	KeeICE.KP->LaunchGroupEditor(NS_ConvertUTF16toUTF8(uniqueID).get());
	return NS_OK;
}

NS_IMETHODIMP CKeeFox::LaunchLoginEditor(const nsAString &uniqueID)
{
	KeeICE.KP->LaunchLoginEditor(NS_ConvertUTF16toUTF8(uniqueID).get());
	return NS_OK;
}