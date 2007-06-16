<?
function leer_feed($url)
{
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
	{
		foreach ( $feed->get_items() as $item )
		{
			echo "<br> Fecha de la noticia:".$item->get_date('j M Y');
			echo "<br><a href='".$item->get_permalink()."'> Titulo:".$item->get_title()."</a>";
			echo "<br> Descripccion::".$item->get_description();
	
	// 			if ($enclosure = $item->get_enclosure(0)) 
	// 			{
	// 
	// 				// Use the embed() method to embed the enclosure into the page inline.
	// 				echo '<div align="center">';
	// 				echo '<p>' . $enclosure->embed(array(
	// 					'audio' => './for_the_demo/place_audio.png',
	// 					'video' => './for_the_demo/place_video.png',
	// 					'alt' => '<img src="./for_the_demo/mini_podcast.png" class="download" border="0" title="Download the Podcast (' . $enclosure->get_extension() . '; ' . $enclosure->get_size() . ' MB)" />',
	// 					'altclass' => 'download'
	// 				)) . '</p>';
	// 				echo '<p class="footnote" align="center">(' . $enclosure->get_type() . '; ' . $enclosure->get_size() . ' MB)</p>';
	// 				echo '</div>';
	// 				exit;
	// 			}
			echo "<br>";
		}
	}
	else
		echo "Entro, pero no hay nada!";
	
}

function obtener_feed($url)
{
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