<?php
include("xmlrpc/lib/xmlrpc.inc");

class clienteXmlRPC
{
	private $client;
			
	public function __construct( $port = 8080)
	{
		$this->client = new xmlrpc_client("/RPC2", "localhost", $port);
		$this->client->setDebug(1);
	}
	
	public function crear_mensaje_array($vector,$tipo,$nombre_funcion)
	{
		$arrayVal = array();
		$i = 0;
		foreach ( $vector as $valor )
		{
			$arrayVal[$i] = new xmlrpcval($valor, $tipo);
			$i++;
		}

		$vector = new xmlrpcval($arrayVal,"array");

		$f=new xmlrpcmsg($nombre_funcion,array($vector));
		
		if ( $f )
			return $f;
		else
			echo "Error al crear mensaje";
	}
	
	public function mandar($mensaje)
	{
		$r=&$this->client->send($mensaje);
		if( !$r->faultCode() )
		{
			echo "<br>El resultado es".$r->value()->scalarval()."<br>";
			$v=$r->value();
			print "</pre><br/>State number " . $stateno . " is "
				. htmlspecialchars($v->scalarval()) . "<br/>";
			//devuelvo la respuesta.
			return $r;
		}
		else
		{
			print "An error occurred: ";
			print "Code: " . htmlspecialchars($r->faultCode())
				. " Reason: '" . htmlspecialchars($r->faultString()) . "'</pre><br/>";
			return false;
		}
	}
}

?>