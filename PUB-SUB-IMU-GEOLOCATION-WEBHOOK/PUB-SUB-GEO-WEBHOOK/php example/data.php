<?php
	// ASSUME GET REQUEST
   if( $_GET["lat"] || $_GET["lon"] ) {

	$data =  $_GET['device-name'] .",";
	$data .=  $_GET['lat'] .",";
	$data .=  $_GET['lon'] .",";
	$data .=  $_GET['accu'] .",";
	$data .=  $_GET['accel-x'] .",";
	$data .=  $_GET['accel-y'] .",";
	$data .=  $_GET['accel-z'] .",";
	$data .=  $_GET['magnetom-x'] .",";
	$data .=  $_GET['magnetom-y'] .",";
	$data .=  $_GET['magnetom-z'] .",";
	$data .=  $_GET['gyro-x'] .",";
	$data .=  $_GET['gyro-y'] .",";
	$data .=  $_GET['gyro-z'] .",";
	$data .=  $_GET['yaw'] .",";
	$data .=  $_GET['pitch'] .",";
	$data .=  $_GET['roll'] .",";
	$data .=  $_GET['magnitude'] .",";
    $data .=  date("F j, Y, g:i a") . "\n";

//to append -. use a
      $fp = fopen('data.txt', 'a') or die("Unable to open file!");
      fwrite($fp, $data );
      fclose($fp);

      exit();
   }
?>
<html>
   <body>
		Your HTML/JS Here. 
   </body>
</html>
