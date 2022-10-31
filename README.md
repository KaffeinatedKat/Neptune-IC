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

## Profiles
Neptune-IC uses profiles to save login information for users, profiles are stored in `profiles.json` and currently must be manually added. Examples are provided in `profilesExample.json`

Logging in with a profile is as simple as `login [name-of-user]`

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

If you place JSON files from Infinite Campus in the `userJson/` directory of Neptune-IC, the program can parse those instead of logging you in and fetching them from the Infinite Campus servers. These JSON responces can be harvested with inspect element

The first JSON you need is a GET request to `https://your-infinitecampus-url/campus/resources/portal/grades` put this in `userJson/[profile_name]_grades.json`

The second JSON is a GET request to `https://your-infinitecampus-url/campus/api/portal/students`, put this in `userJson/[profile_name]_students.json`

For each class, there will be a GET request with your classes section_id which can all be found in `grades.json`. Place each one in `userJson/[profile_name]_[section_id].json`

After you get all the json files, simply run `profiles create [profile_name]` and use `json` as the login method

A profile with JSON Login
```json
"exampleUser":{
  "login_method":"json",
}
```

#### (Please note that this was mainly created for testing purposes and is not the intended way to use Neptune-IC)
