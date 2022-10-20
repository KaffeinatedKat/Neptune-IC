#pragma once
#include <iostream>
#include <string>

std::string profileAllHelp() {
    return "<Profile Help>\n\n"
"Profiles are how you login to your different infinite campus accounts\n"
"For information on how to create profiles, refer to the Neptune-IC\n"
"github page at https://github.com/KaffeinatedKat/Neptune-IC\n\n";
}

std::string microsoftProfileHelp() {
    return "\t[b] to go back\n\n"
           "[IC URL]: This is the URL for your schools infinite campus page\n"
           "\n[IC Login URL Path]: This is the path of your IC URL that redirects to the microsoft login page\n"
           "\n[SAMLResponce]: This is a long string that is POSTed to the login path, this can be extracted with inspect element\n";
}
