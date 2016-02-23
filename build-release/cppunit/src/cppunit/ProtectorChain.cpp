#include "ProtectorChain.h"

CPPUNIT_NS_BEGIN


class ProtectorChain::ProtectFunctor : public Functor
{
public:
  ProtectFunctor( Protector *protector,
                  const Functor &functor,
                  const ProtectorContext &context )
      : m_protector( protector )
      , m_functor( functor )
      , m_context( context )
  {
  }

  bool operator()() const
  {
    return m_protector->protect( m_functor, m_context );
  }

private:
  Protector *m_protector;
  const Functor &m_functor;
  const ProtectorContext &m_context;
};


ProtectorChain::~ProtectorChain()
{
  while ( count() > 0 )
    pop();
}


void 
ProtectorChain::push( Protector *protector )
{
  m_protectors.push_back( protector );
}


void 
ProtectorChain::pop()
{
  delete m_protectors.back();
  m_protectors.pop_back();
}

int 
ProtectorChain::count() const
{
  return m_protectors.size();
}


bool 
ProtectorChain::protect( const Functor &functor,
                         const ProtectorContext &context )
{
  if ( m_protectors.empty() )
    return functor();

  Functors functors;
  for ( int index = m_protectors.size()-1; index >= 0; --index )
  {
    const Functor &protectedFunctor = 
              functors.empty() ? functor : *functors.back();

    functors.push_back( new ProtectFunctor( m_protectors[index],
                                            protectedFunctor, 
                                            context ) );
  }

  const Functor &outermostFunctor = *functors.back();
  bool succeed = outermostFunctor();

  for ( unsigned int deletingIndex = 0; deletingIndex < m_protectors.size(); ++deletingIndex )
    delete functors[deletingIndex];

  return succeed;
}


CPPUNIT_NS_END
