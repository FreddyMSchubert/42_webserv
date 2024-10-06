# Webserv
## Overview
### [Official Requirements](#official-mandatory-requirements)
### [Visual Flowchart for Webserv](#visual-flowchart-for-webserv)

### Official (mandatory) requirements
#### What we need to do:
1. Take a config file or have a default one
2. The Webservers must not be blocking and bounce properly (what that means is: handle client disconnects, response timeouts, connection management)
3. **Must** use only 1 poll() **for all** I/0 operations and while using poll() it must check read and write at the same time (poll() is a function that can monitor multiple fds and is able to do non-blocking input/output operations. It can also respond to multiple events like incoming data or cennection request as they happen to handle many connections simultaneousely)
4. HTTP status codes need to be correct
5. Server must have default error pages if none are provided
6. We must be able to serve a fully static website
7. Clients must be able to upload files
8. We need GET, POST, DELETE methods
9. The Server must be able to listen to multiple ports at the same time

#### What is forbidden (just to have a general overview):
1. We are not allowed to execve another web server
2. Checking the value of errno is forbidden *after* reading/writing operation
3. We are not allowed to use poll() before reading
4. Fork is **only** allowed for CGI and **nothing else** (look into Visual Flowchart for Webserv for CGI explanation)

#### General Notes that wouldnt fit in do or donts:
1. We are allowed to use every makro and define like FD_SET, FD_CLR, FD_ISSET, FD_ZERO
2. Requests shouldnt hang forever
3. NGINX und HTTP 1.1 will be used to compare our webserver against

!**HAVENT LISTED THE REQUIREMENTS FOR THE CONFIG FILE LOOK IN THE SUBJECT TO LEARN MORE**!

### Visual Flowchart for Webserv
![image](https://github.com/user-attachments/assets/c95149d8-faa0-4bcf-bc12-ba82dbd0efcf)

Questions I had with this Flowchart (maybe this also helps u guys)
|Question I had|Answer|
|-|-|
|What is CGI exactly?|CGI short for Common Gateway Interface is a Tool that helps serve interactive and dynamic web applications. We as the Webserver basically allow our web application to execute external programs or scripts in response to user requests.|
|What is 'URI'?|A URI is a combination of a URL and the path to a specific resource on the host (something like a file or a directory).|
|What is an auto-index?|An auto-indexed Website is triggered if the User requests a directory where no index file (index.html or index.php etc) is present. It activates when there is no default file to serve in this directory. For that case we can show the folder structure itself (doesnt need advanced styling etc.pp just "raw" page that allowes the user to navigate trough the directorys until he finds something that we can serve).|
