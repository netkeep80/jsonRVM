#pragma once
#include "interface.h"
#include "json.hpp"


namespace rm
{
	using namespace std;
	using namespace nlohmann;

	class database_api : public interface_t<database_api>
	{
	public:
		virtual void	get_entity(json& ent, const string& ent_id) { this->impl().get_entity(ent, ent_id); }
		virtual void	add_entity(const json& ent, string& ent_id) { this->impl().add_entity(ent, ent_id); }
		virtual void	query_entity(json& ent, const json& query) { this->impl().query_entity(ent, query); }
	};
}

