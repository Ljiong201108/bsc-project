#pragma once

#include <bits/stdc++.h>

// Input file preliminary checks
inline void inputFilePreCheck(std::ifstream& file, size_t &fileSize) {
    if (!file) {
        std::cerr << "Unable to open input file" << std::endl;
        exit(1);
    }

    file.seekg(0, file.end);
    size_t file_size = file.tellg();
    if (file_size == 0) {
        std::cerr << "File is empty!" << std::endl;
        exit(1);
    }
    file.seekg(0, file.beg);
    fileSize=file_size;
}

inline void readFile(const std::string &fileName, std::vector<uint8_t> &in, std::vector<uint8_t> &out){
	std::ifstream inFile(fileName, std::ifstream::binary);
	size_t fileSize;
	inputFilePreCheck(inFile, fileSize);
    in.resize(fileSize);
    out.resize(20 * fileSize);

	inFile.read((char*)in.data(), fileSize);
    inFile.close();
}