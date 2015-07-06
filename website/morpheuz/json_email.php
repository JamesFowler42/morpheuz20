<?php
if (isset($_POST['email']) && 
    isset($_SERVER['HTTP_X_CLIENT_TOKEN']) &&
    $_SERVER["HTTP_X_CLIENT_TOKEN"] == "morpheuz20")  {
   $email = json_decode(urldecode($_POST['email']));
   $to = $email->to;
   $subject = $email->subject;
   $message = "<html><body>";
   $message .= $email->message;
   $message .= "</body></html>";
   $headers = "From: " . $email->from . "\r\n";
   $headers .= "Reply-To: " . $email->from . "\r\n";
   $headers .= "X-Mailer: PHP/" . phpversion() . "\r\n";
   $headers .= "MIME-Version: 1.0\r\n";
   $headers .= "Content-Type: text/html; charset=utf-8\r\n";
   $finalMessage = wordwrap( $message, 75, "\n" );
   $retval = mail ($to,$subject,$finalMessage,$headers);
   if( $retval == true )  
   {
      http_response_code(200);
      echo "Message sent successfully to " . $to;
   }
   else
   {
      http_response_code(500);
   }
} else {
  http_response_code(400);
}
?>
