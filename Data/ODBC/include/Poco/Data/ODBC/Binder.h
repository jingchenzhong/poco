//
// Binder.h
//
// $Id: //poco/Main/Data/ODBC/include/Poco/Data/ODBC/Binder.h#3 $
//
// Library: ODBC
// Package: ODBC
// Module:  Binder
//
// Definition of the Binder class.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef DataConnectors_ODBC_Binder_INCLUDED
#define DataConnectors_ODBC_Binder_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/AbstractBinder.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/ODBC/Handle.h"
#include "Poco/Data/ODBC/Parameter.h"
#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/Data/ODBC/TypeInfo.h"
#include "Poco/DateTime.h"
#include "Poco/Exception.h"
#include <vector>
#include <map>
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqlext.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API Binder: public Poco::Data::AbstractBinder
	/// Binds placeholders in the sql query to the provided values. Performs data types mapping.
{
public:
	typedef AbstractBinder::Direction Direction;
	typedef std::map<SQLPOINTER, SQLLEN> ParamMap;

	static const size_t DEFAULT_PARAM_SIZE = 1024;

	enum ParameterBinding
	{
		PB_IMMEDIATE,
		PB_AT_EXEC
	};

	Binder(const StatementHandle& rStmt,
		ParameterBinding dataBinding = PB_IMMEDIATE,
		TypeInfo* pDataTypes = 0);
		/// Creates the Binder.

	~Binder();
		/// Destroys the Binder.

	void bind(std::size_t pos, const Poco::Int8& val, Direction dir = PD_IN);
		/// Binds an Int8.

	void bind(std::size_t pos, const Poco::UInt8& val, Direction dir = PD_IN);
		/// Binds an UInt8.

	void bind(std::size_t pos, const Poco::Int16& val, Direction dir = PD_IN);
		/// Binds an Int16.

	void bind(std::size_t pos, const Poco::UInt16& val, Direction dir = PD_IN);
		/// Binds an UInt16.

	void bind(std::size_t pos, const Poco::Int32& val, Direction dir = PD_IN);
		/// Binds an Int32.

	void bind(std::size_t pos, const Poco::UInt32& val, Direction dir = PD_IN);
		/// Binds an UInt32.

	void bind(std::size_t pos, const Poco::Int64& val, Direction dir = PD_IN);
		/// Binds an Int64.

	void bind(std::size_t pos, const Poco::UInt64& val, Direction dir = PD_IN);
		/// Binds an UInt64.

	void bind(std::size_t pos, const bool& val, Direction dir = PD_IN);
		/// Binds a boolean.

	void bind(std::size_t pos, const float& val, Direction dir = PD_IN);
		/// Binds a float.

	void bind(std::size_t pos, const double& val, Direction dir = PD_IN);
		/// Binds a double.

	void bind(std::size_t pos, const char& val, Direction dir = PD_IN);
		/// Binds a single character.

	void bind(std::size_t pos, const std::string& val, Direction dir = PD_IN);
		/// Binds a string.

	void bind(std::size_t pos, const Poco::Data::BLOB& val, Direction dir = PD_IN);
		/// Binds a BLOB.

	void bind(std::size_t pos, const Poco::DateTime& val, Direction dir = PD_IN);
		/// Binds a DateTime.

	void setDataBinding(ParameterBinding binding);
		/// Set data binding type.

	ParameterBinding getDataBinding() const;
		/// Return data binding type.

	std::size_t parameterSize(SQLPOINTER pAddr) const;
		/// Returns bound data size for parameter at specified position.

	void synchronize();
		/// Transfers the results of non-POD outbound parameters from internal 
		/// holders back into the externally supplied buffers.

private:
	typedef std::vector<SQLLEN*> LengthVec;
	typedef std::map<SQL_TIMESTAMP_STRUCT*, DateTime*> TimestampMap;
	typedef std::map<char*, std::string*> StringMap;

	void describeParameter(std::size_t pos);
		/// Sets the description field for the parameter, if needed.

	void bind(std::size_t pos, const char* const &pVal, Direction dir = PD_IN);
		/// Binds a const char ptr. 
		/// This is a private no-op in this implementation
		/// due to security risk.

	std::size_t getParamSize(std::size_t pos);
		/// Returns parameter size as defined by data source.
		/// Used to determine buffer size for variable size out-bound parameters 
		/// (string and BLOB).

	SQLSMALLINT getParamType(Direction dir) const;
		/// Returns ODBC parameter type based on the parameter binding direction
		/// specified by user.

	template <typename T>
	void bindImpl(std::size_t pos, T& val, SQLSMALLINT cDataType, Direction dir)
	{
		_lengthIndicator.push_back(0);

		SQLINTEGER colSize = 0;
		SQLSMALLINT decDigits = 0;
		if (_pTypeInfo)
		{
			try
			{
				colSize = _pTypeInfo->getInfo(cDataType, "COLUMN_SIZE");
				decDigits = _pTypeInfo->getInfo(cDataType, "MINIMUM_SCALE");
			}catch (NotFoundException&) { }
		}

		if (Utility::isError(SQLBindParameter(_rStmt, 
			(SQLUSMALLINT) pos + 1, 
			getParamType(dir), 
			cDataType, 
			Utility::sqlDataType(cDataType), 
			colSize,
			decDigits,
			(SQLPOINTER) &val, 
			0, 
			0)))
		{
			throw StatementException(_rStmt, "SQLBindParameter()");
		}
	}

	const StatementHandle& _rStmt;
	LengthVec _lengthIndicator;
	ParamMap _inParams;
	ParamMap _outParams;
	ParameterBinding _paramBinding;
	TimestampMap _timestamps;
	StringMap _strings;
	const TypeInfo* _pTypeInfo;
};


//
// inlines
//
inline void Binder::bind(std::size_t pos, const Poco::Int8& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_STINYINT, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt8& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_UTINYINT, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::Int16& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_SSHORT, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt16& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_USHORT, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt32& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_ULONG, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::Int32& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_SLONG, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt64& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_UBIGINT, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::Int64& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_SBIGINT, dir);
}


inline void Binder::bind(std::size_t pos, const float& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_FLOAT, dir);
}


inline void Binder::bind(std::size_t pos, const double& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_DOUBLE, dir);
}


inline void Binder::bind(std::size_t pos, const bool& val, Direction dir)
{
	bindImpl(pos, val, Utility::boolDataType, dir);
}


inline void Binder::bind(std::size_t pos, const char& val, Direction dir)
{
	bindImpl(pos, val, SQL_C_STINYINT, dir);
}


inline void Binder::setDataBinding(Binder::ParameterBinding binding)
{
	_paramBinding = binding;
}


inline Binder::ParameterBinding Binder::getDataBinding() const
{
	return _paramBinding;
}


} } } // namespace Poco::Data::ODBC


#endif // DataConnectors_ODBC_Binder_INCLUDED
