#ifndef BRA_GATE_CONTROLLED_U1_HPP
# define BRA_GATE_CONTROLLED_U1_HPP

# include <string>
# include <iosfwd>

# include <bra/gate/gate.hpp>
# include <bra/state.hpp>


namespace bra
{
  namespace gate
  {
    class controlled_u1 final
      : public ::bra::gate::gate
    {
     public:
      using qubit_type = ::bra::state::qubit_type;
      using control_qubit_type = ::bra::state::control_qubit_type;
      using real_type = ::bra::state::real_type;

     private:
      real_type phase_;
      qubit_type target_qubit_;
      control_qubit_type control_qubit_;

      static std::string const name_;

     public:
      controlled_u1(
        real_type const phase,
        qubit_type const target_qubit,
        control_qubit_type const control_qubit);

      ~controlled_u1() = default;
      controlled_u1(controlled_u1 const&) = delete;
      controlled_u1& operator=(controlled_u1 const&) = delete;
      controlled_u1(controlled_u1&&) = delete;
      controlled_u1& operator=(controlled_u1&&) = delete;

     private:
      ::bra::state& do_apply(::bra::state& state) const override;
      std::string const& do_name() const override;
      std::string do_representation(
        std::ostringstream& repr_stream, int const parameter_width) const override;
    }; // class controlled_u1
  } // namespace gate
} // namespace bra


#endif // BRA_GATE_CONTROLLED_U1_HPP
