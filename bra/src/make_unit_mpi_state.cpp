#ifndef BRA_NO_MPI
# include <string>
# include <sstream>
# include <ios>
# include <vector>
# include <memory>

# include <yampi/communicator.hpp>
# include <yampi/environment.hpp>

# include <bra/make_unit_mpi_state.hpp>
# include <bra/state.hpp>
# include <bra/unit_mpi_state.hpp>


namespace bra
{
  std::unique_ptr< ::bra::state > make_unit_mpi_state(
    ::bra::state::state_integer_type const initial_integer,
    ::bra::state::bit_integer_type const num_local_qubits,
    ::bra::state::bit_integer_type const num_unit_qubits,
    ::bra::state::bit_integer_type const total_num_qubits,
    unsigned int const num_threads_per_process,
    unsigned int const num_processes_per_unit,
    ::bra::state::seed_type const seed,
    yampi::communicator const& communicator,
    yampi::environment const& environment)
  {
    return std::unique_ptr< ::bra::state >{
      new ::bra::unit_mpi_state{
        initial_integer, num_local_qubits, num_unit_qubits, total_num_qubits,
        num_threads_per_process, num_processes_per_unit, seed, communicator, environment}};
  }

  std::unique_ptr< ::bra::state > make_unit_mpi_state(
    ::bra::state::state_integer_type const initial_integer,
    ::bra::state::bit_integer_type const num_local_qubits,
    ::bra::state::bit_integer_type const num_unit_qubits,
    std::vector< ::bra::state::qubit_type > const& initial_permutation,
    unsigned int const num_threads_per_process,
    unsigned int const num_processes_per_unit,
    ::bra::state::seed_type const seed,
    yampi::communicator const& communicator,
    yampi::environment const& environment)
  {
    return std::unique_ptr< ::bra::state >{
      new ::bra::unit_mpi_state{
        initial_integer, num_local_qubits, num_unit_qubits, initial_permutation,
        num_threads_per_process, num_processes_per_unit, seed, communicator, environment}};
  }
} // namespace bra


#endif // BRA_NO_MPI
