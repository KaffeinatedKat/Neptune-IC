# Neptune-IC
An open source Infinite Campus client for the command line

## Build & Run
```bash
git clone https://github.com/KaffeinatedKat/Neptune-IC
cd Neptune-IC
mkdir build && cd build
cmake ..
make
./neptune
```

## Profiles
Neptune-IC uses profiles to save login information for users, profiles are stored in `profiles.json`

Profiles can be created with the command `profiles create [profile_name]`

Examples are also provided in `profilesExample.json` if you would like to manually add them 

Logging in with a profile is as simple as `login [profile_name]`

# Supported login methods

#### Currently only Student Login is supported at this time
#### This has been tested with 2 school currently and it works identically for both, other schools should also work fine but there is no guarentees

## Microsoft login
#### Microsoft login is "supported" (only tested with 1 school)

Use the login method `microsoft` when creating a profile. It will ask for 3 fields:

- IC URL (the url to your schools infinite campus site)
- IC Login URL Path (the path of the url of the SSO button)
- SAMLResponce (a really long identification string used by microsoft, this can be harvested with inspect element)


A profile with microsoft login:
```json
"exampleUser": {
  "login_method":"microsoft",
  "campus_url":"https://your-infinitecampus-url",
  "login_path":"/path/saml/is/posted/to",
  "saml":"your-microsoft-SAMLResponse"
}
```

## Username/Password Login
#### Username/Password login is also supported (again only tested with 1 school)

Use the login method `username` when creating a profile. It will ask for 3 fields:

- IC URL (the url to your schools infinite campus site)
- Username (your account username)
- Password (your account password)

A profile with username login:
```json
"exampleUser": {
  "login_method":"username",
  "campus_url":"https://your-infinitecampus-url",
  "username": "your-username",
  "password": "your-password"
}
```

## JSON "Login" (Currently brocken)
#### Neptune-IC parses the JSON responces from the Infinite Campus servers, so what I call JSON Login is supported

If you place JSON files from Infinite Campus in the `userJson/` directory of Neptune-IC, the program can parse those instead of logging you in and fetching them from the Infinite Campus servers. These JSON responces can be harvested with inspect element

The first JSON you need is a GET request to `https://your-infinitecampus-url/campus/resources/portal/grades` put this in `userJson/[profile_name]_grades.json`

The second JSON is a GET request to `https://your-infinitecampus-url/campus/api/portal/students`, put this in `userJson/[profile_name]_students.json`

For each class, there will be a GET request with your classes section_id which can all be found in `grades.json`. Place each one in `userJson/[profile_name]_[section_id].json`

After you get all the json files, simply run `profiles create [profile_name]` and use `json` as the login method, it requires no fields

A profile with JSON Login
```json
"exampleUser":{
  "login_method":"json",
}
```

#### (Please note that json login was mainly created for testing purposes and is not the intended way to use Neptune-IC)
