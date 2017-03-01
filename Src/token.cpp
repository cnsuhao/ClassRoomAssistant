#include "token.h"
#include "ConfigFile.h"

strToken::tokenTable_type strToken::getInstance()
{
	return m_instance;
}



TokenSystem<std::string, std::string> strToken::m_instance;


std::string user_list::passwd;
std::string user_list::user_name;
std::string user_list::login_user;
std::string user_list::login_passwd;
std::string user_list::cgi;
std::string user_list::ip;
std::string user_list::server_ip;
std::string user_list::server_port;
void user_list::init(const std::string configure_file)
{
	ConfigFile cfg(configure_file);
	passwd = cfg.getValue("passwd", "accout");
	user_name = cfg.getValue("user", "accout");
	cgi = cfg.getValue("cgi", "accout");
	ip = cfg.getValue("login_ip", "accout");
	server_ip = cfg.getValue("server_ip", "server");
	server_port = cfg.getValue("port", "server");
	if (passwd.empty())
	{
		passwd = "dmtlb";
		cfg.addValue("passwd", passwd, "accout");
	}
	if (user_name.empty())
	{
		user_name = "admin";
		cfg.addValue("user", user_name, "accout");
	}
	if (cgi.empty())
	{
		cgi = "demo.cgi?";
		cfg.addValue("cgi", cgi, "accout");
	}
	if (server_ip.empty())
	{
		server_ip = "192.168.8.83";
		cfg.addValue("server_ip", server_ip, "server");
	}
	if(server_port.empty ())
	{
		server_port = "4507";
		cfg.addValue("port", server_port, "server");
	}
	cfg.save();
}