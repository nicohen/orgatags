<html>
<head>
<TITLE></TITLE>
<link href="formato.css" rel="stylesheet" type="text/css">
<script language="JavaScript">
function cargar_feed()
{
	alert(document.formu.feedURL.value);
	window.parent.principal.location.href = 'cargar_feed.php?feedurl=' + document.formu.feedURL.value;
}
function ocultar_mostrar(cual)
{
	var tabla;
	tabla = document.getElementById(cual);
	if(tabla.style.display == 'none')
	{
		tabla.style.display = 'block';
	}
	else
	{
		tabla.style.display  = 'none';
	}
}
function ocultar(cual)
{
	document.getElementById(cual).style.display = 'none';
}
</script>
</head>
<body bgcolor="#B4DBFF">

<form name="formu" method="POST">

<table align="left" width="70%">
<TR>
<td align="center">
	<table width="100%" border="0" align="center" cellpadding="2" cellspacing="0" bgcolor="#BFFFB2">
	<TR>
		<TD colspan="3" class="letraNegraMedia">Ingrese la url del feed</TD>
	</TR>
	<TR>
		<TD width="50%" align="left"><input id="feedURL" name="feedURL" type="text" size="100"></TD>
		<TD width="50%" align="left"><input type="button" value="agregar" class="boton" onmouseover="this.className='botonSel'" onmouseout="this.className='boton'" onfocus="this.className='botonSel'" onblur="this.className='boton'" onclick="javascript:cargar_feed()"></TD>
	</TR>
	</table>
</td>
</table>

<br>
<br>
<br>
<hr>
<br>
<?
// include_once("cargar_feed.php");
//Los feeds que hay quye mostrar.(pedirlos por xmlrpc)
if ( $feed )
{
	$feed = urldecode($feed);
	echo "FEEEDDDSS: $feed";
// 	obtener_noticias();
}
else
{
	$a;
	echo "NOOOOOOOOOOOO   FEEEDDDSS: $feed";
}
?>
<table width="50%" border="0" align="left" cellpadding="0" cellspacing="0" bgcolor="#47A49E">
<TR onclick="javascript:ocultar_mostrar('tabla1')">
	<td><img src="images/plus.gif" width="14" height="14" hspace="0" vspace="0"></td>
	<TD class="letraNegra">t&iacute;tulo de una noticia</TD>
</TR>
<TR>
	<td colspan="2">
		<table id="tabla1" name="tabla1" bgcolor="#5A73FF">
		<TR>
			<TD class="letraNegra">Descripcion de la noticia, aca puede haber una descripcion de la noticia que a lo mejor no dice nada interesante, pero tiene que estar al pedo, como esto.</TD>
		</TR>
		<table>
	</td>
</TR>
</table>

<?
$paraOcultar = "ocultar('tabla1')";
echo "<script language=\"javascript\">$paraOcultar;</script>"; ?>
<p>http://www.clarin.com/diario/hoy/um/sumariorss.xml</p>
</form>
</body>
</html>
