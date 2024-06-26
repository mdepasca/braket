#include <string>
#include <ios>
#include <iomanip>
#include <sstream>

#include <ket/qubit_io.hpp>
#include <ket/control_io.hpp>

#include <bra/gate/gate.hpp>
#include <bra/gate/adj_controlled_u2.hpp>
#include <bra/state.hpp>


namespace bra
{
  namespace gate
  {
    std::string const adj_controlled_u2::name_ = "CU2+";

    adj_controlled_u2::adj_controlled_u2(
      real_type const phase1, real_type const phase2,
      qubit_type const target_qubit, control_qubit_type const control_qubit)
      : ::bra::gate::gate{}, phase1_{phase1}, phase2_{phase2}, target_qubit_{target_qubit}, control_qubit_{control_qubit}
    { }

    ::bra::state& adj_controlled_u2::do_apply(::bra::state& state) const
    { return state.adj_controlled_u2(phase1_, phase2_, target_qubit_, control_qubit_); }

    std::string const& adj_controlled_u2::do_name() const { return name_; }
    std::string adj_controlled_u2::do_representation(
      std::ostringstream& repr_stream, int const parameter_width) const
    {
      repr_stream
        << std::right
        << std::setw(parameter_width) << control_qubit_
        << std::setw(parameter_width) << target_qubit_
        << std::setw(parameter_width) << phase1_
        << std::setw(parameter_width) << phase2_;
      return repr_stream.str();
    }
  } // namespace gate
} // namespace bra
