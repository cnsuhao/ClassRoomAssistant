#pragma once
#include <string>
#include <map>
#include <windows.h>

template<typename unique_id, typename type_value>
class TokenSystem
{
public:
	TokenSystem();
	~TokenSystem();
	typedef unique_id key_type;
	typedef type_value value_type;
	void update_table(key_type id, value_type value);
	bool is_exist(key_type id);
	value_type operator()(key_type id);
	value_type getValue(key_type id);
	value_type operator[](key_type id);
	static std::map<key_type, value_type>token_table;
};

template<typename unique_id, typename type_value>
std::map<unique_id, type_value> TokenSystem<unique_id, type_value>::token_table;

template<typename unique_id, typename type_value>
TokenSystem<unique_id, type_value>::TokenSystem<unique_id, type_value>()
{

}

template<typename unique_id, typename type_value>
TokenSystem<unique_id, type_value>::~TokenSystem<unique_id, type_value>()
{
}
template<typename unique_id, typename type_value>
type_value TokenSystem<unique_id, type_value>::operator()(unique_id id)
{
	return token_table[id];
}
template<typename unique_id, typename type_value>
type_value TokenSystem<unique_id, type_value>::operator[](unique_id id)
{
	return token_table[id];
}
template<typename unique_id, typename type_value>
type_value TokenSystem<unique_id, type_value>::getValue(key_type id)
{
	return token_table[id];
}
template<typename unique_id, typename type_value>
void TokenSystem<unique_id, type_value>::update_table(unique_id id, type_value value)
{
	token_table[id] = value;
}
template<typename unique_id, typename type_value>
bool TokenSystem<unique_id, type_value>::is_exist(key_type id)
{
	return (token_table.find(id) != token_table.end());
}


class strToken
{
public:
	typedef TokenSystem<std::string, std::string> tokenTable_type;
	static TokenSystem<std::string, std::string> getInstance();
private:
	static TokenSystem<std::string, std::string> m_instance;
};


class user_list
{
public:
	static void init(const std::string configure_file);
	static std::string user_name;
	static std::string passwd;
	static std::string login_user;
	static std::string login_passwd;
	static std::string cgi;
	static std::string ip;
	static std::string server_ip;
	static std::string server_port;
};