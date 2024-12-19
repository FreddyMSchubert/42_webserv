# todo

- [ ] make get_location + get_locaitons use paths, not strings (should accept both filepaths and paths) F
- [ ] make filepath locations possible, setting settings only for one specific file F
	- what do we do if file is pointing to a folder? do we then do directory listing if you look for that file? lol F
---
- [ ] CGI PATH_INFO thing (i dont know what it is but do that) N
- [x] what are server_names and do we need to do anything with them? -> server_names how the adress of the website. Works as follows: 1. Website gets static IP 2. Domain name (server_name) points to that IP via DNS record 3. when user types domain name dns resolver looks up ip and the request goes to that ip. And no we do not need to do anything with it, test it like this: curl -H "Host: test1.com" http://localhost:<port>
- [x] actually implement redirections & error pages -> implemented and also caught an overflow which is also fixed
---
- [ ] run testers and check that they run through. A
	- not actually required but maybe it finds somethign deasastrous, evals might use them as well
- [ ] go through eval sheet A
- [ ] test using siege A
