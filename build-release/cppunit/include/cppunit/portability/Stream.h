#ifndef CPPUNIT_PORTABILITY_STREAM_H_INCLUDED
#define CPPUNIT_PORTABILITY_STREAM_H_INCLUDED

// This module define:
// Type CppUT::Stream (either std::stream or a custom type)
// Type CppUT::OStringStream (eitjer std::ostringstream, older alternate or a custom type)
// Functions stdCOut() & stdCErr() which returns a reference on cout & cerr stream (or our
// custom stream).

#include <cppunit/Portability.h>


#if defined( CPPUNIT_NO_STREAM )

#include <string>
#include <stdio.h>
#include <string.h>

CPPUNIT_NS_BEGIN

class StreamBuffer
{
public:
   virtual ~StreamBuffer() {}

   virtual void write( const char *text, unsigned int length ) = 0;

   virtual void flush() {}
};


class StringStreamBuffer : public StreamBuffer
{
public:
   std::string str() const
   {
      return str_;
   }

public: // overridden from StreamBuffer
   void write( const char *text, unsigned int length )
   {
      str_.append( text, length );
   }

private:
   std::string str_;
};


class FileStreamBuffer : public StreamBuffer
{
public:
   FileStreamBuffer( FILE *file )
      : file_( file )
   {
   }

   FILE *file() const
   {
      return file_;
   }

public: // overridden from StreamBuffer
   void write( const char *text, unsigned int length )
   {
      if ( file_ )
         fwrite( text, sizeof(char), length, file_ );
   }

   void flush() 
   {
      if ( file_ )
         fflush( file_ );
   }

private:
   FILE *file_;
};


class OStream
{
public:
   OStream()
      : buffer_( 0 )
   {
   }

   OStream( StreamBuffer *buffer )
      : buffer_( buffer )
   {
   }

   virtual ~OStream()
   {
     flush();
   }

   OStream &flush()
   {
	   if ( buffer_ )
		    buffer_->flush();
	   return *this;
   }

   void setBuffer( StreamBuffer *buffer )
   {
      buffer_ = buffer;
   }

   OStream &write( const char *text, unsigned int length )
   {
      if ( buffer_ )
         buffer_->write( text, length );
      return *this;
   }

   OStream &write( const char *text )
   {
      return write( text, strlen(text) );
   }

   OStream &operator <<( bool v )
   {
      const char *out = v ? "true" : "false";
      return write( out );
   }

   OStream &operator <<( short v )
   {
      char buffer[64];
      sprintf( buffer, "%hd", v );
      return write( buffer );
   }

   OStream &operator <<( unsigned short v )
   {
      char buffer[64];
      sprintf( buffer, "%hu", v );
      return write( buffer );
   }

   OStream &operator <<( int v )
   {
      char buffer[64];
      sprintf( buffer, "%d", v );
      return write( buffer );
   }

   OStream &operator <<( unsigned int v )
   {
      char buffer[64];
      sprintf( buffer, "%u", v );
      return write( buffer );
   }

   OStream &operator <<( long v )
   {
      char buffer[64];
      sprintf( buffer, "%ld", v );
      return write( buffer );
   }

   OStream &operator <<( unsigned long v )
   {
      char buffer[64];
      sprintf( buffer, "%lu", v );
      return write( buffer );
   }

   OStream &operator <<( float v )
   {
      char buffer[128];
      sprintf( buffer, "%.16g", double(v) );
      return write( buffer );
   }

   OStream &operator <<( double v )
   {
      char buffer[128];
      sprintf( buffer, "%.16g", v );
      return write( buffer );
   }

   OStream &operator <<( long double v )
   {
      char buffer[128];
      sprintf( buffer, "%.16g", double(v) );
      return write( buffer );
   }

   OStream &operator <<( const void *v )
   {
      char buffer[64];
      sprintf( buffer, "%p", v );
      return write( buffer );
   }

   OStream &operator <<( const char *v )
   {
      return write( v ? v : "NULL" );
   }

   OStream &operator <<( char c )
   {
      char buffer[16];
      sprintf( buffer, "%c", c );
      return write( buffer );
   }

   OStream &operator <<( const std::string &s )
   {
      return write( s.c_str(), s.length() );
   }

private:
   StreamBuffer *buffer_;
};


class OStringStream : public OStream
{
public:
	OStringStream()
		: OStream( &buffer_ )
	{
	}

	std::string str() const
	{
		return buffer_.str();
	}

private:
	StringStreamBuffer buffer_;
};


class OFileStream : public OStream
{
public:
   OFileStream( FILE *file )
      : OStream( &buffer_ )
      , buffer_( file )
      , ownFile_( false )
   {
   }

   OFileStream( const char *path )
      : OStream( &buffer_ )
      , buffer_( fopen( path, "wt" ) )
      , ownFile_( true )
   {
   }

   virtual ~OFileStream()
   {
      if ( ownFile_  &&  buffer_.file() )
         fclose( buffer_.file() );
   }

private:
   FileStreamBuffer buffer_;
   bool ownFile_;
};

inline OStream &stdCOut() 
{
   static OFileStream stream( stdout );
   return stream;
}

inline OStream &stdCErr() 
{
   static OFileStream stream( stderr );
   return stream;
}

CPPUNIT_NS_END

#elif CPPUNIT_HAVE_SSTREAM // #if defined( CPPUNIT_NO_STREAM )
# include <sstream>
# include <fstream>

    CPPUNIT_NS_BEGIN
    typedef std::ostringstream OStringStream;      // The standard C++ way
    typedef std::ofstream OFileStream;
    CPPUNIT_NS_END


#elif CPPUNIT_HAVE_CLASS_STRSTREAM
# include <string>
# if CPPUNIT_HAVE_STRSTREAM
#   include <strstream>
# else // CPPUNIT_HAVE_STRSTREAM
#  include <strstream.h>
# endif // CPPUNIT_HAVE_CLASS_STRSTREAM

    CPPUNIT_NS_BEGIN

    class OStringStream : public std::ostrstream 
    {
    public:
        std::string str()
        {
//            (*this) << '\0';
//            std::string msg(std::ostrstream::str());
//            std::ostrstream::freeze(false);
//            return msg;
// Alternative implementation that don't rely on freeze which is not
// available on some platforms:
            return std::string( std::ostrstream::str(), pcount() );
        }
    };

    CPPUNIT_NS_END
#else // CPPUNIT_HAVE_CLASS_STRSTREAM
#   error Cannot define CppUnit::OStringStream.
#endif // #if defined( CPPUNIT_NO_STREAM )



#if !defined( CPPUNIT_NO_STREAM )

#include <iostream>

    CPPUNIT_NS_BEGIN

    typedef std::ostream OStream;

    inline OStream &stdCOut() 
    {
        return std::cout;
    }

    inline OStream &stdCErr() 
    {
       return std::cerr;
    }

    CPPUNIT_NS_END
   
#endif // #if !defined( CPPUNIT_NO_STREAM )

#endif // CPPUNIT_PORTABILITY_STREAM_H_INCLUDED

