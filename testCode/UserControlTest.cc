#include "../oj_server/oj_control.hpp"
#include "../oj_server/oj_model2.hpp"
using namespace ns_model;
int main()
{
    ns_control::UserControl userControl;
    const std::string name = "user1";
    const std::string password = "12345";
    
    const std::string token = "c94d5164703ec1189d2d";
    User* user = new User(name, password, token);

    cout<<userControl.LoginUser(name,password);

    return 0;
}
