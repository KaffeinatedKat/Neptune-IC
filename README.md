# Neptune-IC
An open source Infinite Campus client for the command line

## Build
```bash
git clone https://github.com/KaffeinatedKat/Neptune-IC
cd Neptune-IC
mkdir build && cd build
cmake ..
make
```

# Supported login methods

#### Currently only Student Login is supported at this time

## Microsoft login
#### Microsoft login is "supported", only tested with 1 school

A profile with microsoft login:
```json
"exampleUser": {
  "login_method":"microsoft",
  "campus_url":"https://your-infinitecampus-url",
  "login_path":"/path/saml/is/posted/to",
  "saml":"your-microsoft-SAMLResponse"
}
```
The SAMLResponse can be harvested via inspect element when logging into Infinite Campus, it is in the POST request after a GET to `login.microsoftonline.com`

`login_path` is the URL path the SAMLResponse is POSTed to


## JSON "Login"
#### Neptune-IC parses the JSON responces from the Infinite Campus servers, so what I call JSON Login is supported

If you place JSON files from Infinite Campus in the root directory of Neptune-IC, the program can parse those instead of logging you in and fetching them from the Infinite Campus servers. These JSON responces can be harvested with inspect element

The first JSON you need is a GET request to `https://your-infinitecampus-url/campus/resources/portal/grades`

The second JSON you need is a GET request to `https://your-infinitecampus-url/campus/api/portal/students`

A profile with JSON Login
```json
"exampleUser":{
  "login_method":"json_file",
  "grades_json":"/path/to/grades.json",
  "student_json":"/path/to/students.json"
}
```

#### (Please note that this was mainly created for testing purposes and is not the intended way to use Neptune-IC)
