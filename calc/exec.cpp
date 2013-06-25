/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.hpp"

#include "error.hpp"

#include "exec.hpp"

namespace calc {

class Wrapper {
public:
    explicit Wrapper(const program & p)
        : impl_(p) {}

    variable operator()(void * ctx, variable * stack) const
    {
        try {
            return impl_(ctx, stack);
        } catch(calc::comparison_exception & e) {
            throw calc::exec_exception() << mstd::make_error_message((boost::wformat(L"Failed to compare %s and %s, using '%s'") % e.lhs_type().name() %
                            e.rhs_type().name() % e.op()).str());
        } catch(calc::division_by_zero_exception &) {
            throw calc::exec_exception() << mstd::error_message("Division by zero");
        } catch(boost::exception & e) {
            throw calc::exec_exception(e);
        } catch(std::exception &) {
            throw calc::exec_exception() << mstd::error_message("Unknown error");
        }
    }
private:
    program impl_;
};

program wrap(const program & p)
{
    return Wrapper(p);
}

}
