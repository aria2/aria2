#ifndef CPPUNIT_PROTECTORCHAIN_H
#define CPPUNIT_PROTECTORCHAIN_H

#include <cppunit/Protector.h>
#include <cppunit/portability/CppUnitDeque.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif


CPPUNIT_NS_BEGIN

/*! \brief Protector chain (Implementation).
 * Implementation detail.
 * \internal Protector that protect a Functor using a chain of nested Protector.
 */
class CPPUNIT_API ProtectorChain : public Protector
{
public:
  ~ProtectorChain();

  void push( Protector *protector );

  void pop();

  int count() const;

  bool protect( const Functor &functor,
                const ProtectorContext &context );

private:
  class ProtectFunctor;

private:
  typedef CppUnitDeque<Protector *> Protectors;
  Protectors m_protectors;

  typedef CppUnitDeque<Functor *> Functors;
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_PROTECTORCHAIN_H

