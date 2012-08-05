//
// FileChannel.h
//
// $Id: //poco/1.4/Foundation/include/Poco/FileChannel.h#3 $
//
// Library: Foundation
// Package: Logging
// Module:  FileChannel
//
// Definition of the FileChannel class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
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


#ifndef Foundation_FileChannel_INCLUDED
#define Foundation_FileChannel_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Channel.h"
#include "Poco/Timestamp.h"
#include "Poco/Mutex.h"


namespace Poco {


class LogFile;
class RotateStrategy;
class ArchiveStrategy;
class PurgeStrategy;


class Foundation_API FileChannel: public Channel
	/// A Channel that writes to a file. This class supports
	/// flexible log file rotation and archiving, as well
	/// as automatic purging of archived log files.
	///
	/// Only the message's text is written, followed
	/// by a newline.
	///
	/// Chain this channel to a FormattingChannel with an
	/// appropriate Formatter to control what is in the text. 
	///
	/// The FileChannel support log file rotation based
	/// on log file size or time intervals.
	/// Archived log files can be compressed in gzip format.
	/// Older archived files can be automatically deleted
	/// (purged).
	///
	/// The rotation strategy can be specified with the
	/// "rotation" property, which can take one of the
	/// follwing values:
	///
	///   * never:         no log rotation
	///   * [day,][hh]:mm: the file is rotated on specified day/time
	///                    day - day is specified as long or short day name (Monday|Mon, Tuesday|Tue, ... );
	///                          day can be omitted, in which case log is rotated every day
	///                    hh  - valid hour range is 00-23;
	///                          hour can be omitted, in which case log is rotated every hour
	///                    mm  - valid minute range is 00-59;
	///                          minute must be specified
	///   * daily:         the file is rotated daily
	///   * weekly:        the file is rotated every seven days
	///   * monthly:       the file is rotated every 30 days
	///   * <n> minutes:   the file is rotated every <n> minutes, 
	///                    where <n> is an integer greater than zero.
	///   * <n> hours:     the file is rotated every <n> hours, where
	///                    <n> is an integer greater than zero.
	///   * <n> days:      the file is rotated every <n> days, where
	///                    <n> is an integer greater than zero.
	///   * <n> weeks:     the file is rotated every <n> weeks, where
	///                    <n> is an integer greater than zero.
	///   * <n> months:    the file is rotated every <n> months, where
	///                    <n> is an integer greater than zero and
	///                    a month has 30 days.
	///   * <n>:           the file is rotated when its size exceeds
	///                    <n> bytes.
	///   * <n> K:         the file is rotated when its size exceeds
	///                    <n> Kilobytes.
	///   * <n> M:         the file is rotated when its size exceeds
	///                    <n> Megabytes.
	///
	/// NOTE: For periodic log file rotation (daily, weekly, monthly, etc.),
	/// the date and time of log file creation or last rotation is
	/// written into the first line of the log file. This is because
	/// there is no reliable way to find out the real creation date of
	/// a file on many platforms (e.g., most Unix platforms do not
	/// provide the creation date, and Windows has its own issues
	/// with its "File System Tunneling Capabilities").
	///
	/// Using the "archive" property it is possible to specify
	/// how archived log files are named. The following values
	/// for the "archive" property are supported:
	///
	///   * number:     A number, starting with 0, is appended to
	///                 the name of archived log files. The newest
	///                 archived log file always has the number 0.
	///                 For example, if the log file is named
	///                 "access.log", and it fulfils the criteria
	///                 for rotation, the file is renamed to
	///                 "access.log.0". If a file named "access.log.0"
	///                 already exists, it is renamed to "access.log.1",
	///                 and so on.
	///   * timestamp:  A timestamp is appended to the log file name.
	///                 For example, if the log file is named
	///                 "access.log", and it fulfils the criteria
	///                 for rotation, the file is renamed to
	///                 "access.log.20050802110300".
	///
	/// Using the "times" property it is possible to specify
	/// time mode for the day/time based rotation. The following values
	/// for the "times" property are supported:
	///
	///   * utc:        Rotation strategy is based on UTC time (default).
	///   * local:      Rotation strategy is based on local time.
	///
	/// Archived log files can be compressed using the gzip compression
	/// method. Compressing can be controlled with the "compress"
	/// property. The following values for the "compress" property
	/// are supported:
	///
	///   * true:       Compress archived log files.
	///   * false:      Do not compress archived log files.
	///
	/// Archived log files can be automatically purged, either if
	/// they reach a certain age, or if the number of archived
	/// log files reaches a given maximum number. This is 
	/// controlled by the purgeAge and purgeCount properties.
	///
	/// The purgeAge property can have the following values:
	///
	///   * <n> [seconds]: the maximum age is <n> seconds.
	///   * <n> minutes:   the maximum age is <n> minutes.
	///   * <n> hours:     the maximum age is <n> hours.
	///   * <n> days:      the maximum age is <n> days.
	///   * <n> weeks:     the maximum age is <n> weeks.
	///   * <n> months:    the maximum age is <n> months, where a month has 30 days.
	///
	/// The purgeCount property has an integer value that
	/// specifies the maximum number of archived log files.
	/// If the number is exceeded, archived log files are 
	/// deleted, starting with the oldest.
	///
	/// For a more lightweight file channel class, see SimpleFileChannel.
{
public:
	FileChannel();
		/// Creates the FileChannel.

	FileChannel(const std::string& path);
		/// Creates the FileChannel for a file with the given path.

	void open();
		/// Opens the FileChannel and creates the log file if necessary.
		
	void close();
		/// Closes the FileChannel.

	void log(const Message& msg);
		/// Logs the given message to the file.
		
	void setProperty(const std::string& name, const std::string& value);
		/// Sets the property with the given name. 
		/// 
		/// The following properties are supported:
		///   * path:       The log file's path.
		///   * rotation:   The log file's rotation mode. See the 
		///                 FileChannel class for details.
		///   * archive:    The log file's archive mode. See the
		///                 FileChannel class for details.
		///   * times:      The log file's time mode. See the
		///                 FileChannel class for details.
		///   * compress:   Enable or disable compression of
		///                 archived files. See the FileChannel class
		///                 for details.
		///   * purgeAge:   Maximum age of an archived log file before
		///                 it is purged. See the FileChannel class for
		///                 details.
		///   * purgeCount: Maximum number of archived log files before
		///                 files are purged. See the FileChannel class
		///                 for details.

	std::string getProperty(const std::string& name) const;
		/// Returns the value of the property with the given name.
		/// See setProperty() for a description of the supported
		/// properties.

	Timestamp creationDate() const;
		/// Returns the log file's creation date.
		
	UInt64 size() const;
		/// Returns the log file's current size in bytes.

	const std::string& path() const;
		/// Returns the log file's path.

	static const std::string PROP_PATH;
	static const std::string PROP_ROTATION;
	static const std::string PROP_ARCHIVE;
	static const std::string PROP_TIMES;
	static const std::string PROP_COMPRESS;
	static const std::string PROP_PURGEAGE;
	static const std::string PROP_PURGECOUNT;

protected:
	~FileChannel();
	void setRotation(const std::string& rotation);
	void setArchive(const std::string& archive);
	void setCompress(const std::string& compress);
	void setPurgeAge(const std::string& age);
	void setPurgeCount(const std::string& count);
	void purge();

private:
	std::string      _path;
	std::string      _times;
	std::string      _rotation;
	std::string      _archive;
	bool             _compress;
	std::string      _purgeAge;
	std::string      _purgeCount;
	LogFile*         _pFile;
	RotateStrategy*  _pRotateStrategy;
	ArchiveStrategy* _pArchiveStrategy;
	PurgeStrategy*   _pPurgeStrategy;
	FastMutex        _mutex;
};


} // namespace Poco


#endif // Foundation_FileChannel_INCLUDED
