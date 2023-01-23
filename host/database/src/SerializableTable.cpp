#include "SerializableTable.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assert.h>

uint32_t SerializableTable::maxSizeChunk=16*1024*1024;

SerializableTable::SerializableTable(std::string tableName) : 
	tableName(tableName), tempDir(get_current_dir_name()), colChunkSizes(), colNames(), chunkIdx(-1), table(nullptr), mm(nullptr)
{}

SerializableTable::SerializableTable(std::string tableName, std::string tempDir) : 
	tableName(tableName), tempDir(tempDir), colChunkSizes(), colNames(), chunkIdx(-1), table(nullptr), mm(nullptr)
{}

void SerializableTable::addCol(std::string colName, uint32_t typeLength){
	nameToIdx[colName]=colNames.size();
	colNames.push_back(colName);
	colTypeLength.push_back(typeLength);
}

void SerializableTable::addDataToCol(std::string colName, void *payload, uint32_t num){
	char *ptr=(char*)payload;
	uint32_t colIdx=nameToIdx[colName];
	uint32_t lastChunkIdx=colChunkSizes[colIdx].size()-1;
	uint32_t lastChunkSize=colChunkSizes[colIdx][lastChunkIdx];
	if(lastChunkSize<maxSizeChunk){
		uint32_t canWrite=std::min(num, maxSizeChunk-lastChunkSize);
		colChunkSizes[colIdx][lastChunkIdx]+=canWrite;

		std::fstream fs;
		std::stringstream filename;
		filename<<tempDir<<"/"<<tableName<<"/"<<colName<<"/"<<lastChunkIdx;
		fs.open(filename.str(), std::fstream::out | std::fstream::app);
		fs.write(ptr, colTypeLength[colIdx]*num);
		fs.close();

		ptr+=canWrite;
		num-=canWrite;
		lastChunkIdx++;
	}

	while(num){
		uint32_t canWrite=std::min(num, maxSizeChunk);
		colChunkSizes[colIdx].push_back(canWrite);

		std::fstream fs;
		std::stringstream filename;
		filename<<tempDir<<"/"<<tableName<<"/"<<colName<<"/"<<lastChunkIdx;
		fs.open(filename.str(), std::fstream::out | std::fstream::app);
		fs.write(ptr, colTypeLength[colIdx]*num);
		fs.close();

		ptr+=canWrite;
		num-=canWrite;
		lastChunkIdx++;
	}
}

xf::database::gqe::Table& SerializableTable::chunk(uint32_t idx){
	assert(0<=idx && idx<getChunkNumber());

	if(idx==chunkIdx) return *table;

	if(table!=nullptr) delete table;
	if(mm!=nullptr) delete mm;

	table=new xf::database::gqe::Table;
	mm=new xf::database::gqe::utils::MM;

	for(int i=0;i<colNames.size();i++){
		std::fstream fs;
		std::stringstream filename;
		filename<<tempDir<<"/"<<tableName<<"/"<<colNames[i]<<"/"<<idx;
		fs.open(filename.str(), std::fstream::in);

		if(colTypeLength[i]==1){
			bool *ptr_col=mm->aligned_alloc<bool>(colChunkSizes[i][idx]);
			fs.read((char*)ptr_col, sizeof(bool)*colChunkSizes[i][idx]);
			table->addCol(colNames[i], xf::database::gqe::TypeEnum::TypeBool, ptr_col, colChunkSizes[i][idx]);
		}else if(colTypeLength[i]==4){
			int32_t *ptr_col=mm->aligned_alloc<int32_t>(colChunkSizes[i][idx]);
			fs.read((char*)ptr_col, sizeof(int32_t)*colChunkSizes[i][idx]);
			table->addCol(colNames[i], xf::database::gqe::TypeEnum::TypeInt32, ptr_col, colChunkSizes[i][idx]);
		}else if(colTypeLength[i]==8){
			int64_t *ptr_col=mm->aligned_alloc<int64_t>(colChunkSizes[i][idx]);
			fs.read((char*)ptr_col, sizeof(int64_t)*colChunkSizes[i][idx]);
			table->addCol(colNames[i], xf::database::gqe::TypeEnum::TypeInt64, ptr_col, colChunkSizes[i][idx]);
		}

		fs.close();
	}

	return *table;
}

uint32_t SerializableTable::getChunkNumber(){
	uint32_t size;
	for(int i=0;i<colChunkSizes.size();i++){
		if(i==0) size=colChunkSizes[0].size();
		else assert(size==colChunkSizes[i].size() && "chunk size should be equal");
	}
	return colChunkSizes[0].size();
}