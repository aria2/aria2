#ifndef COMHelper_h
#define COMHelper_h

#pragma warning( push)
#pragma warning( disable: 4786)
#pragma warning( disable: 4290)

#include <new>
#include <typeinfo>

#include <comdef.h>
#include <comip.h>

namespace COMUtility
{

// simple template to reduce the typing effort when doing reinterpret<void**> casts
template< typename Interface>
  inline void** ppvoid( Interface **ppInterface)
{
  return reinterpret_cast< void**>( ppInterface);
}


template< typename Interface>
  inline GUID* piid( const Interface* pInterface)
{
  return __uuidof( pInterface);
}

// to be used with _com_ptr_t, uses function overloading
template< typename InterfacePtr>
  inline GUID* piid( const InterfacePtr& pInterface)
{
  return const_cast< GUID*>( &pInterface.GetIID());
}


template< typename Interface>
  inline const GUID& riid( const Interface* pInterface)
{
  return *__uuidof( pInterface);
}

// to be used with _com_ptr_t, use function overloading
template< typename InterfacePtr>
  inline const GUID& riid( const InterfacePtr& pInterface)
{
  return pInterface.GetIID();
}


// this is used for regular COM interface pointers
template < class rawTargetInterface, class rawSourceInterface>
  inline rawTargetInterface* interface_cast( rawSourceInterface* pSrcInterface) throw( std::bad_cast)
{
  rawTargetInterface* pTargetInterface = NULL;

  if ( SUCCEEDED( pSrcInterface->QueryInterface( __uuidof( pTargetInterface),
                                                 reinterpret_cast<void**>(&pTargetInterface))))
    return pTargetInterface;
  else
    throw std::bad_cast();
}


// non-throwing versions of the same - need to use parameter std::nothrow on function call
template < class rawTargetInterface, class rawSourceInterface>
  inline rawTargetInterface* interface_cast( const std::nothrow_t&,
                                             rawSourceInterface* pSrcInterface) throw()
{
  rawTargetInterface* pTargetInterface = NULL;

  pSrcInterface->QueryInterface( __uuidof( pTargetInterface),
                                 reinterpret_cast<void**>(&pTargetInterface));
  return pTargetInterface;
}


// Is probably best used as a static member of the class, so it's
// accessible everywhere
class COMExceptionThrower
{
public:
	COMExceptionThrower( void){}
	~COMExceptionThrower( void){}

	COMExceptionThrower( const HRESULT errCode)
	{
		this->operator=( errCode);
	}
		
protected:
	// don't allow regular copy constructor call
	COMExceptionThrower( const COMExceptionThrower&);

public:
	inline const COMExceptionThrower& operator=( const HRESULT errCode) const throw( _com_error)
	{
		if ( FAILED( errCode))
			_com_raise_error( errCode);
			
		return *this;
	}
};

} // end namespace COMUtility

#pragma warning( pop) 

#endif // COMHelper_h
