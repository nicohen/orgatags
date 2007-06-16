#include <cassert>
#include <stdexcept>
#include <iostream>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

using namespace std;

class sampleAddMethod : public xmlrpc_c::method 
{
public:
	sampleAddMethod()
	{
		// signature and help strings are documentation -- the client
		// can query this information with a system.methodSignature and
		// system.methodHelp RPC.
		this->_signature = "i:ii";
		// method's result and two arguments are integers
		this->_help = "This method adds two integers together";
	}
	void execute(xmlrpc_c::paramList const& paramList,xmlrpc_c::value *   const  retvalP)
	{

		std::vector<xmlrpc_c::value> array(paramList.getArray(0));
		string concat;
		//concateno en un strin el vector que viene
		cout << endl << "tamaño del array: " << array.size() << endl;
		for( int i = 0 ; i < array.size() ; i++ )
		{
			xmlrpc_c::value_string const param1(array[i]);
    			string const param1Value(static_cast<string>(param1));
			concat += static_cast<string>(param1Value);
			cout << "Recibi: " << static_cast<string>(param1Value) << endl;
		}

		//devuelvo el mismo vector que recibi, solo para probar
		*retvalP = xmlrpc_c::value_array(array);
	}
};

class sampleAddMethod2 : public xmlrpc_c::method 
{
public:
	sampleAddMethod2()
	{
		this->_signature = "i:ii";
		this->_help = "This method adds two integers together";
	}
	void execute(xmlrpc_c::paramList const& paramList,xmlrpc_c::value *   const  retvalP)
	{
		string const addend(paramList.getString(0));
		string const adder(paramList.getString(1));
		
		//Asi se ve la cantidad de parametros que vienen
		int tamanio = paramList.size();

		//Con esto se puede verificar y devolver error si la cantidad de parametros no es 2
// 		paramList.verifyEnd(2);
	
		//concateno los 2 string que recibo.
		*retvalP = xmlrpc_c::value_string(addend + adder);
	}
};

int main(int const,const char ** const)
{

	try
	{
		xmlrpc_c::registry myRegistry;
	
		//aca se agragan las clases
		xmlrpc_c::methodPtr const sampleAddMethodP(new sampleAddMethod);
		xmlrpc_c::methodPtr const sampleAddMethodP2(new sampleAddMethod2);
	
		//aca se agragan los metodos
		myRegistry.addMethod("sample.add", sampleAddMethodP);
		myRegistry.addMethod("sample.add2", sampleAddMethodP2);
	
		xmlrpc_c::serverAbyss myAbyssServer(myRegistry,8080,"/tmp/xmlrpc_log");
	
		myAbyssServer.run();
		// xmlrpc_c::serverAbyss.run() never returns
		assert(false);
	} 
	catch (exception const& e) 
	{
		cerr << "Something failed.  " << e.what() << endl;
	}
	return 0;
}
