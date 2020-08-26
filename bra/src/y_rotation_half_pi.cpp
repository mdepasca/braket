#include <string>
#include <ios>
#include <iomanip>
#include <sstream>

#include <ket/qubit_io.hpp>

#include <bra/gate/gate.hpp>
#include <bra/gate/y_rotation_half_pi.hpp>
#include <bra/state.hpp>


namespace bra
{
  namespace gate
  {
    std::string const y_rotation_half_pi::name_ = "+Y";

    y_rotation_half_pi::y_rotation_half_pi(qubit_type const qubit)
      : ::bra::gate::gate{}, qubit_{qubit}
    { }

    ::bra::state& y_rotation_half_pi::do_apply(::bra::state& state) const
    { return state.y_rotation_half_pi(qubit_); }

    std::string const& y_rotation_half_pi::do_name() const { return name_; }
    std::string y_rotation_half_pi::do_representation(
      std::ostringstream& repr_stream, int const parameter_width) const
    {
      repr_stream
        << std::right
        << std::setw(parameter_width) << qubit_;
      return repr_stream.str();
    }
  } // namespace gate
} // namespace bra
