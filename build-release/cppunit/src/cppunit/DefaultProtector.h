#ifndef CPPUNIT_DEFAULTPROTECTOR_H
#define CPPUNIT_DEFAULTPROTECTOR_H

#include <cppunit/Protector.h>

CPPUNIT_NS_BEGIN

/*! \brief Default protector that catch all exceptions (Implementation).
 *
 * Implementation detail.
 * \internal This protector catch and generate a failure for the following
 * exception types:
 * - Exception
 * - std::exception
 * - ...
 */
class DefaultProtector : public Protector
{
public:
  bool protect( const Functor &functor,
                const ProtectorContext &context );
};

CPPUNIT_NS_END

#endif // CPPUNIT_DEFAULTPROTECTOR_H

