# Proccess_Overseer
Server that receives through sockets a processes to queue, assigns thread to run it then monitors it and finally sends back result to controller

Arguments to pass to the server/overseer:
overseer <port>
  
  <port> overseer port
   
Arguments to pass to the server/overseer:
controller <address> <port> [-o out_file] [-log log_file] [-t seconds] <file> [arg...]
  
  <address> overseer IP address
  <port> overseer port
  <file> the file to be executed
  [arg...] an arbitrary quantity of arguments passed to file
  --help prints usage message
  [-o out_file] redirects stdout and stderr of the executed <file> to out_file
  [-log log_file] redirects stdout of the overseer’s management of <file> to log_file
  [-t seconds] specifies the timeout for SIGTERM to be sent
  
  < > angle brackets indicate required arguments.
  [ ] brackets indicate optional arguments.
  ... ellipses indicate an arbitrary quantity of arguments
  
