#include <string>
#include <ios>
#include <iomanip>
#include <sstream>

#include <ket/qubit_io.hpp>
#include <ket/control_io.hpp>

#include <bra/gate/gate.hpp>
#include <bra/gate/adj_controlled_s_gate.hpp>
#include <bra/state.hpp>


namespace bra
{
  namespace gate
  {
    std::string const adj_controlled_s_gate::name_ = "CS+";

    adj_controlled_s_gate::adj_controlled_s_gate(
      complex_type const& phase_coefficient,
      qubit_type const target_qubit, control_qubit_type const control_qubit)
      : ::bra::gate::gate{},
        phase_coefficient_{phase_coefficient},
        target_qubit_{target_qubit},
        control_qubit_{control_qubit}
    { }

    ::bra::state& adj_controlled_s_gate::do_apply(::bra::state& state) const
    { return state.adj_controlled_phase_shift(phase_coefficient_, target_qubit_, control_qubit_); }

    std::string const& adj_controlled_s_gate::do_name() const { return name_; }
    std::string adj_controlled_s_gate::do_representation(
      std::ostringstream& repr_stream, int const parameter_width) const
    {
      repr_stream
        << std::right
        << std::setw(parameter_width) << control_qubit_
        << std::setw(parameter_width) << target_qubit_;
      return repr_stream.str();
    }
  } // namespace gate
} // namespace bra
