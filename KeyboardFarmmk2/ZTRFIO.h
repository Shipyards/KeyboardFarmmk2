#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "ZTRobject.h"
#include "serializeable.h"



namespace ZTRengine
{
	class ZTRFIO
	{
	private:
		fstream Fstrm;
		ios::streampos lastreadpos;
		ios::streampos lastwritepos;
		ios::streampos laststrmpos;
		void create_file()
		{
			this->Fstrm.open(this->filename, ios::out);
			this->Fstrm.close();
		}
	public:
		enum to_frommode
		{
			standard,
			endpos,
			beginpos,
			currentpos,
			lastpos
		};
		std::string filename;
		bool OK;
		ZTRFIO(std::string filename)
		{
			using namespace std;
			if (filename[filename.length() - 4] != '.')
			{
				filename += ".bin";
			}
			this->filename = filename;
			this->Fstrm = fstream(filename, ios::in | ios::out | ios::binary);
			if (this->Fstrm.fail())
			{
				this->Fstrm.close();
				this->create_file();
				this->Fstrm = fstream(filename, ios::in | ios::out | ios::binary);
			}
			this->OK = false;
			this->record_readpos();
			this->record_writepos();
		}
		~ZTRFIO() { this->Fstrm.close(); } 
		void clear_file() // * WARNING * THIS FUNCTION WILL PERMANETLY DELETE FILE CONTENTS
		{
			this->Fstrm.close();
			this->Fstrm.open(this->filename, ios::out);
			this->Fstrm.close();
			this->Fstrm.open(this->filename, ios::in | ios::out | ios::binary);
		}
		bool delete_file()
		{
			this->Fstrm.close();
			this->OK = false;
			remove(this->filename.c_str());
		}
		bool is_empty()
		{
			this->Fstrm.seekg(0, ios::end);
			bool empty = this->Fstrm.tellg() == 0;
			this->ptr_goback();
			return empty;
		}
		void ptr_tostart()
		{
			this->laststrmpos = this->Fstrm.tellg();
			this->Fstrm.seekg(0, ios::beg);
		}
		void ptr_toend()
		{
			this->laststrmpos = this->Fstrm.tellg();
			this->Fstrm.seekg(0, ios::end);
		}
		void ptr_toread()
		{
			this->laststrmpos = this->Fstrm.tellg();
			this->Fstrm.seekg(this->lastreadpos);
		}
		void record_readpos()
		{
			this->lastreadpos = this->Fstrm.tellg();
		}
		void ptr_towrite()
		{
			this->laststrmpos = this->Fstrm.tellg();
			this->Fstrm.seekg(this->lastwritepos);
		}
		void record_writepos()
		{
			this->lastwritepos = this->Fstrm.tellg();
		}
		void ptr_goback()
		{
			this->Fstrm.seekg(this->laststrmpos);
		}
		void record_lastpos()
		{
			this->laststrmpos = this->Fstrm.tellg();
		}
		template<class T>
		void write(T writevar) //only works with basic types
		{
			this->ptr_towrite();
			this->Fstrm.write(reinterpret_cast<char*>(&writevar), sizeof(writevar));
			this->record_writepos();
		}
		template<class T>
		void write(T writevar, to_frommode modeset) //only works with basic types
		{
			this->record_lastpos();
			switch (modeset)
			{
			case endpos:
				this->ptr_toend();
			case beginpos:
				this->ptr_tostart();
			case standard:
				this->ptr_towrite();
			case lastpos:
				this->ptr_goback();
			}
			this->Fstrm.write(reinterpret_cast<char*>(&writevar), sizeof(writevar));
			this->record_writepos();
		}
		template<> void write<std::string>(std::string writevar) //only works with basic types
		{
			this->ptr_towrite();
			size_t sizeofstring = writevar.size();
			this->Fstrm.write(reinterpret_cast<const char*>(&sizeofstring), sizeof(size_t));
			this->Fstrm.write(writevar.c_str(), sizeofstring);
			this->record_writepos();
		}
		template<> void write<std::string>(std::string writevar, to_frommode modeset) //only works with basic types
		{
			size_t sizeofstring = writevar.size();
			this->record_lastpos();
			switch (modeset)
			{
			case endpos:
				this->ptr_toend();
			case beginpos:
				this->ptr_tostart();
			case standard:
				this->ptr_towrite();
			case currentpos:
				break;
			case lastpos:
				this->ptr_goback();
			default:
				throw ZTRutils::ID10T_ERR(); // :)
			}
			this->Fstrm.write(reinterpret_cast<const char*>(&sizeofstring), sizeof(size_t));
			this->Fstrm.write(writevar.c_str(), sizeofstring);
			this->record_writepos();
		}
		template<class T>
		T read(to_frommode modeset) //only works with basic types
		{
			switch (modeset)
			{
			case endpos:
				this->ptr_toend();
			case beginpos:
				this->ptr_tostart();
			case standard:
				this->ptr_toread();
			case currentpos:
				break;
			case lastpos:
				this->ptr_goback();
			default:
				throw ZTRutils::ID10T_ERR(); // :)
			}
			T returnvar = T();  
			this->Fstrm.read((char*)(&returnvar), sizeof(T));
			this->record_readpos();
			return returnvar;
		}
		template<class T>
		T read()
		{
			this->ptr_toread();
			T returnvar = T();
			this->Fstrm.read((char*)(&returnvar), sizeof(T));
			this->record_readpos();
			return returnvar;
		}
		template<> std::string read<std::string>(to_frommode modeset)
		{
			switch (modeset)
			{
			case endpos:
				this->ptr_toend();
			case beginpos:
				this->ptr_tostart();
			case standard:
				this->ptr_toread();
			case currentpos:
				break;
			case lastpos:
				this->ptr_goback();
			default:
				throw ZTRutils::ID10T_ERR(); // :)
			}
			size_t size;
			this->Fstrm.read(reinterpret_cast<char*>(&size), sizeof(size_t));
			std::string holdstring(size,'\0');
			this->Fstrm.read(&holdstring[0], size);
			this->record_readpos();
			return holdstring;
		}
		template<> std::string read<std::string>()
		{
			this->ptr_toread();
			size_t size;  
			this->Fstrm.read(reinterpret_cast<char*>(&size), sizeof(size_t));
			std::string holdstring(size, '\0');
			this->Fstrm.read(&holdstring[0], size);
			this->record_readpos();
			return holdstring;
		}
	};
}