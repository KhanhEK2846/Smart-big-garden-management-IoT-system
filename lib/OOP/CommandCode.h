//HTTP Response Code
#define Continue_Code 100
#define Received_Code 200
#define Init_Gateway_Code 202
#define No_Content_Code 204
#define Bad_Request_Code 400
#define Forbidden_Code 403
#define Not_Found_Code 404
#define Gone_Code 410
#define No_Response_Code 444
#define Network_Authentication_Required 511
//MD5
#define Init_Node_Code "45ef6ef895e5639e9f8f06c686d46e5c"
#define Default "7a1920d61156abc05a60135aefe8bc67" //Node -> Gateway
#define Broadcast "be55b6387170df0ca68f41225268e842" // Send both except from sender
#define Infection "f0ddc0838281faf6d55e2cf840a2a8ef" // Handle command & Broadcast