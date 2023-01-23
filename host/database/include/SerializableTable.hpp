#include "xf_database/gqe_table.hpp"
#include "xf_database/gqe_utils.hpp"
#include <map>

class SerializableTable{
	std::string tableName;
	std::vector<std::vector<uint32_t>> colChunkSizes;
	std::vector<std::string> colNames;
	std::vector<uint32_t> colTypeLength;
	std::map<std::string, uint32_t> nameToIdx;
	std::string tempDir;

	int chunkIdx;
	xf::database::gqe::Table *table;
	xf::database::gqe::utils::MM *mm;

	static uint32_t maxSizeChunk;

	SerializableTable(std::string tableName);
	SerializableTable(std::string tableName, std::string tempDir);

	void addCol(std::string colName, uint32_t typeLength);
	void addDataToCol(std::string colName, void *payload, uint32_t num);

	xf::database::gqe::Table& chunk(uint32_t idx);
	uint32_t getChunkNumber();
};