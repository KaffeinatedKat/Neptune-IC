try:
    from seleniumwire import webdriver
except:
    print("Please install seleniumwire, this can be done with pip\n'pip install selenium-wire'")
    exit()

url = input("What is the url of the SSO button on your infinite campus page? ")

try:
    driver = webdriver.Chrome()
except:
    try:
        driver = webdriver.Firefox()
    except:
        print("Please install firefox or chrome to use this tool\n")
        exit()

newString = ""


driver.get(url)

while True:
    for request in driver.requests:
        if request.response:
            if b'SAMLResponse' in request.body:
                for x in request.body.decode().split('%'):
                    if x.startswith("S"):
                        newString += x
                        continue
                    newString += bytes.fromhex(x[:2].lower()).decode() + x[2:]
                print(newString[13:newString.rfind("&")])
                driver.close()
                exit()


