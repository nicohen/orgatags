<?
include_once("cliente_xmlrpc.php");

$feed = obtener_feed($feedurl);

if ( $feed )
{
	//creo el cliente.
	$cliente = new clienteXmlRPC(8080);
	if ( $feed->data )
	{
		$feeds = array();
		$noticias = array();
		$i = 0;
		foreach ( $feed->get_items() as $item )
		{
			echo "lei una noticia<br>";
			//Fecha de la noticia
			$fecha = $item->get_date('j M Y');
			//Link de la noticia
			$link = $item->get_permalink();
			//Titulo
			$titulo = $item->get_title();
			//Descripccion
			$descripcion = $item->get_description();

			$feeds[$i]['fecha_noticia'] = $item->get_date('j M Y');
			$feeds[$i]['link_noticia'] = $item->get_permalink();
			$feeds[$i]['titulo_noticia'] = $item->get_title();
			$feeds[$i]['descripcion_noticia'] = $item->get_description();
			
			$notica_string = "$fecha/fintag/$link/fintag/$titulo/fintag/$descripcion";
			$noticias[$i] = $notica_string;
			$i++;
		}
		$mensaje = $cliente->crear_mensaje_array($noticias,"string","sample.add");
		if ( $mensaje )
			echo "mensaje creado correctamente";
		else
			echo "Error al crear el mensaje";
		$cliente->mandar($mensaje);
		exit;
		echo "<script language=\"JavaScript\"> ventana = window.parent.principal;
							ventana.location.href='principal.php?feed=11';</script>";
	}
// 	else
// 		echo "<br><br>Hubo un error al leer el feed";
}
else
	echo "<br><br>Hubo un error al leer el feed";


function obtener_feed($url)
{
// 	error_reporting(0);
	include_once('./SimplePie/simplepie.inc');
	include_once('./SimplePie/idn/idna_convert.class.php');
	
	$feed = new SimplePie();
	$feed->strip_ads(false);
	
	
	if (get_magic_quotes_gpc())
	{
		$url = stripslashes($url);
	}
		
	$feed->feed_url($url);
	$feed->init();
	$feed->handle_content_type();
	
	if ( $feed->data )
		return $feed;
	else
		return false;
}

?>