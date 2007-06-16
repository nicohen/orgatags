<html>
<head><title>xmlrpc</title></head>
<body>
<h1>Getstatename demo</h1>
<h2>Send a U.S. state number to the server and get back the state name</h2>
<h3>The code demonstrates usage of the php_xmlrpc_encode function</h3>
<?php
	include("../../lib/xmlrpc.inc");

	// Play nice to PHP 5 installations with REGISTER_LONG_ARRAYS off
	if(!isset($HTTP_POST_VARS) && isset($_POST))
	{
		$HTTP_POST_VARS = $_POST;
	}

	if(isset($HTTP_POST_VARS["stateno"]) && $HTTP_POST_VARS["stateno"]!="")
	{
		$stateno=(integer)$HTTP_POST_VARS["stateno"];

		//Crear un array
		$arrayVal = array();
		$arrayVal[0] = new xmlrpcval(" Hola", "string");
		$arrayVal[1] = new xmlrpcval(" que", "string");
		$arrayVal[2] = new xmlrpcval(" tal", "string");
		$arrayVal[3] = new xmlrpcval(".", "string");
		$arraytyp = "array";
		$vector = new xmlrpcval($arrayVal,$arraytyp);


// 		$f=new xmlrpcmsg('sample.add',array(new xmlrpcval(5, "int"),new xmlrpcval(10, "int")));
// 		$f=new xmlrpcmsg('sample.add',array(new xmlrpcval("Hola que ", "string"),new xmlrpcval("tal", "string")));
		if ( $cual == 1)
			$f=new xmlrpcmsg('sample.add',array($vector));
		else
			$f=new xmlrpcmsg('sample.add2',array(new xmlrpcval("Hola que ", "string"),new xmlrpcval("tal", "string")));
		
		print "<pre>Sending the following request:\n\n" . htmlentities($f->serialize()) . "\n\nDebug info of server data follows...\n\n";
		//$c=new xmlrpc_client("/server.php", "phpxmlrpc.sourceforge.net", 80);
		$c=new xmlrpc_client("/RPC2", "localhost", 8080);
		$c->setDebug(1);
		$r=&$c->send($f);
		if( !$r->faultCode() )
		{
			###si recibo un array lo veo asi###
			################################
			if ( $cual == 1)
			{
				for ( $w = 0 ; $w < $r->value()->arraySize() ; $w++)
				{
					echo "<br>array : ".$r->value()->arrayMem($w)->scalarval();
				}
			}
			################################
			################################

			echo "<br>El resultado es".$r->value()->scalarval()."<br>";
			$v=$r->value();
			print "</pre><br/>State number " . $stateno . " is "
				. htmlspecialchars($v->scalarval()) . "<br/>";
			// print "<HR>I got this value back<BR><PRE>" .
			//  htmlentities($r->serialize()). "</PRE><HR>\n";
		}
		else
		{
			print "An error occurred: ";
			print "Code: " . htmlspecialchars($r->faultCode())
				. " Reason: '" . htmlspecialchars($r->faultString()) . "'</pre><br/>";
		}
	}
	else
	{
		$stateno = "";
	}

	print "<form action=\"client.php\" method=\"POST\">
<input name=\"stateno\" value=\"" . $stateno . "\"><input type=\"submit\" value=\"go\" name=\"submit\"></form>
<p>Enter a state number to query its name</p>";

?>
<hr/>
<em>$Id: client.php,v 1.7 2006/02/05 17:05:27 ggiunta Exp $</em>
</body>
</html>
