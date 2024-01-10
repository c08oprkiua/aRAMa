#ifndef CFILE_HPP_
#define CFILE_HPP_

#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gctypes.h>
#include "../common/fs_defs.h"

class CFile {
public:
	enum eOpenTypes {
		ReadOnly,
		WriteOnly,
		ReadWrite,
		Append
	};

	CFile();

	CFile(const std::string &filepath, eOpenTypes mode);

	CFile(const uint8_t *memory, int memsize);

	virtual ~CFile();

	int open(const std::string &filepath, eOpenTypes mode);

	int open(const uint8_t *memory, int memsize);

	bool isOpen() const {
		if (iFd >= 0)
			return true;

		if (mem_file)
			return true;

		return false;
	}

	void close();

	int read(uint8_t *ptr, size_t size);

	int write(const uint8_t *ptr, size_t size);

	int fwrite(const char *format, ...);

	int seek(long int offset, int origin);

	u64 tell() {return pos;};

	u64 size() {return filesize;};

	void rewind() {this->seek(0, SEEK_SET);};

protected:
	int iFd;
	const uint8_t *mem_file;
	u64 filesize;
	u64 pos;
};

#endif