#pragma once
#include "interface.h"
#include "json.hpp"


namespace rm
{
	using namespace std;
	using namespace nlohmann;

	template<class database_impl_t>
	class database_api : public interface_t<database_impl_t>
	{
	public:
		void	get_entity(json& ent, const string& ent_id) { this->impl().get_entity(ent, ent_id); }
		void	add_entity(const json& ent, string& ent_id) { this->impl().add_entity(ent, ent_id); }
		void	query_entity(json& ent, const json& query) { this->impl().query_entity(ent, query); }
	};
}

