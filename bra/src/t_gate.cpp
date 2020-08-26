#include <string>
#include <ios>
#include <iomanip>
#include <sstream>

#include <ket/qubit_io.hpp>

#include <bra/gate/gate.hpp>
#include <bra/gate/t_gate.hpp>
#include <bra/state.hpp>


namespace bra
{
  namespace gate
  {
    std::string const t_gate::name_ = "T";

    t_gate::t_gate(complex_type const phase_coefficient, qubit_type const qubit)
      : ::bra::gate::gate{},
        phase_coefficient_{phase_coefficient}, qubit_{qubit}
    { }

    ::bra::state& t_gate::do_apply(::bra::state& state) const
    { return state.phase_shift(phase_coefficient_, qubit_); }

    std::string const& t_gate::do_name() const { return name_; }
    std::string t_gate::do_representation(
      std::ostringstream& repr_stream, int const parameter_width) const
    {
      repr_stream
        << std::right
        << std::setw(parameter_width) << qubit_;
      return repr_stream.str();
    }
  } // namespace gate
} // namespace bra
