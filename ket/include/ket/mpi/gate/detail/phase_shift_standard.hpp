#ifndef KET_MPI_GATE_DETAIL_PHASE_SHIFT_STANDARD_HPP
# define KET_MPI_GATE_DETAIL_PHASE_SHIFT_STANDARD_HPP

# include <boost/config.hpp>

# include <complex>
# include <vector>
# include <array>

# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>

# include <ket/qubit.hpp>
# include <ket/control.hpp>
# ifdef KET_PRINT_LOG
#   include <ket/qubit_io.hpp>
# endif // KET_PRINT_LOG
# include <ket/gate/phase_shift.hpp>
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
#   include <ket/mpi/permutated.hpp>
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/simple_mpi.hpp>
# include <ket/mpi/utility/for_each_local_range.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/gate/detail/append_qubits_string.hpp>
# include <ket/mpi/gate/page/phase_shift_standard.hpp>
# include <ket/mpi/page/is_on_page.hpp>


namespace ket
{
  namespace mpi
  {
    namespace gate
    {
      // phase_shift_coeff
      namespace phase_shift_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Complex, typename Qubit>
        struct call_phase_shift_coeff
        {
          ParallelPolicy parallel_policy_;
          Complex phase_coefficient_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_phase_shift_coeff(
            ParallelPolicy const parallel_policy,
            Complex const& phase_coefficient, ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase_coefficient_{phase_coefficient},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift_coeff(
              parallel_policy_, first, last, phase_coefficient_, permutated_qubit_.qubit());
          }
        }; // struct call_phase_shift_coeff<ParallelPolicy, Complex, Qubit>

        template <typename ParallelPolicy, typename Complex, typename TargetQubit, typename ControlQubit>
        struct call_cphase_shift_coeff
        {
          ParallelPolicy parallel_policy_;
          Complex phase_coefficient_;
          ::ket::mpi::permutated<TargetQubit> permutated_target_qubit_;
          ::ket::mpi::permutated<ControlQubit> permutated_control_qubit_;

          call_cphase_shift_coeff(
            ParallelPolicy const parallel_policy, Complex const& phase_coefficient,
            ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
            ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
            : parallel_policy_{parallel_policy},
              phase_coefficient_{phase_coefficient},
              permutated_target_qubit_{permutated_target_qubit},
              permutated_control_qubit_{permutated_control_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift_coeff(
              parallel_policy_, first, last,
              phase_coefficient_, permutated_target_qubit_.qubit(), permutated_control_qubit_.qubit());
          }
        }; // struct call_cphase_shift_coeff<ParallelPolicy, Complex, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Complex, typename Qubit>
        inline call_phase_shift_coeff<ParallelPolicy, Complex, Qubit>
        make_call_phase_shift_coeff(
          ParallelPolicy const parallel_policy,
          Complex const& phase_coefficient, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase_coefficient, permutated_qubit}; }

        template <typename ParallelPolicy, typename Complex, typename TargetQubit, typename ControlQubit>
        inline call_cphase_shift_coeff<ParallelPolicy, Complex, TargetQubit, ControlQubit>
        make_call_phase_shift_coeff(
          ParallelPolicy const parallel_policy,
          Complex const& phase_coefficient,
          ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
          ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
        { return {parallel_policy, phase_coefficient, permutated_target_qubit, permutated_control_qubit}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        // U1_i(theta)
        // U1_1(theta) (a_0 |0> + a_1 |1>) = a_0 |0> + e^{i theta} a_1 |1>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Complex>
        inline RandomAccessRange& do_phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::phase_shift_coeff(
              parallel_policy, local_state, phase_coefficient, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, &phase_coefficient, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift_coeff(
                parallel_policy, first, last, phase_coefficient, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift_coeff(
              parallel_policy, phase_coefficient, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // CU1_{tc}(theta) or C1U1_{tc}(theta)
        // CU1_{1,2}(theta) (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
        //   = a_{00} |00> + a_{01} |01> + a_{10} |10> + e^{i theta} a_{11} |11>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Complex>
        inline RandomAccessRange& do_phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit)
        {
          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_control_qubit = permutation[control_qubit];
          if (::ket::mpi::page::is_on_page(permutated_target_qubit, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
              return ::ket::mpi::gate::page::cphase_shift_coeff_tcp(
                parallel_policy, local_state, phase_coefficient, permutated_target_qubit, permutated_control_qubit);

            return ::ket::mpi::gate::page::cphase_shift_coeff_tp(
              parallel_policy, local_state, phase_coefficient, permutated_target_qubit, permutated_control_qubit);
          }
          else if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
            return ::ket::mpi::gate::page::cphase_shift_coeff_cp(
              parallel_policy, local_state, phase_coefficient, permutated_target_qubit, permutated_control_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, &phase_coefficient, permutated_target_qubit, permutated_control_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift_coeff(
                parallel_policy, first, last,
                phase_coefficient, permutated_target_qubit.qubit(), permutated_control_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift_coeff(
              parallel_policy, phase_coefficient, permutated_target_qubit, permutated_control_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // C...CU1_{tc...c'}(theta) or CnU1_{tc...c'}(theta)
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename Allocator,
          typename Complex, typename... ControlQubits>
        inline RandomAccessRange& do_phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ControlQubits const... control_qubits)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::phase_shift_coeff(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              phase_coefficient, permutation[target_qubit].qubit(),
              permutation[control_qubit1].qubit(), permutation[control_qubit2].qubit(),
              permutation[control_qubits].qubit()...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
        inline RandomAccessRange& phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift_coeff(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Complex, typename... ControlQubits>
        inline RandomAccessRange& phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift_coeff(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase(coeff) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase(coeff) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase(coeff) "), phase_coefficient),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase(coeff) "), phase_coefficient),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      namespace phase_shift_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using std::conj;
          return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, communicator, environment, conj(phase_coefficient), target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Complex, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Complex const& phase_coefficient,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using std::conj;
          return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, datatype, communicator, environment, conj(phase_coefficient), target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase(coeff)) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase(coeff)) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase(coeff)) "), phase_coefficient),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase(coeff)) "), phase_coefficient),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift_coeff(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename Allocator, typename BufferAllocator,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Complex, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Complex const& phase_coefficient,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase_coefficient, target_qubit, control_qubits...);
      }

      // phase_shift
      namespace phase_shift_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, communicator, environment,
            ::ket::utility::exp_i<complex_type>(phase), target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          return ::ket::mpi::gate::phase_shift_detail::phase_shift_coeff(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, datatype, communicator, environment,
            ::ket::utility::exp_i<complex_type>(phase), target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift(
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift(
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      namespace phase_shift_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          return ::ket::mpi::gate::phase_shift_detail::phase_shift(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, communicator, environment, -phase, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          return ::ket::mpi::gate::phase_shift_detail::phase_shift(
            mpi_policy, parallel_policy,
            local_state, permutation, buffer, datatype, communicator, environment, -phase, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift(
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift(
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase, ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase, target_qubit, control_qubits...);
      }

      // generalized phase_shift
      namespace phase_shift_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Real, typename Qubit>
        struct call_phase_shift2
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_phase_shift2(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift2(
              parallel_policy_, first, last, phase1_, phase2_, permutated_qubit_.qubit());
          }
        }; // struct call_phase_shift2<ParallelPolicy, Real, Qubit>

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        struct call_cphase_shift2
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          ::ket::mpi::permutated<TargetQubit> permutated_target_qubit_;
          ::ket::mpi::permutated<ControlQubit> permutated_control_qubit_;

          call_cphase_shift2(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2,
            ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
            ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              permutated_target_qubit_{permutated_target_qubit},
              permutated_control_qubit_{permutated_control_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift2(
              parallel_policy_, first, last,
              phase1_, phase2_, permutated_target_qubit_.qubit(), permutated_control_qubit_.qubit());
          }
        }; // struct call_cphase_shift2<ParallelPolicy, Real, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Real, typename Qubit>
        inline call_phase_shift2<ParallelPolicy, Real, Qubit>
        make_call_phase_shift2(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase1, phase2, permutated_qubit}; }

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        inline call_cphase_shift2<ParallelPolicy, Real, TargetQubit, ControlQubit>
        make_call_phase_shift2(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2,
          ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
          ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
        { return {parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        // U2_i(theta, theta')
        // U2_1(theta, theta') (a_0 |0> + a_1 |1>) = (a_0 - e^{i theta'} a_1)/sqrt(2) |0> + (e^{i theta} a_0 + e^{i(theta + theta')} a_1)/sqrt(2) |1>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::phase_shift2(
              parallel_policy, local_state, phase1, phase2, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift2(
                parallel_policy, first, last, phase1, phase2, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift2(
              parallel_policy, phase1, phase2, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // CU2_{tc}(theta, theta') or C1U2_{tc}(theta, theta')
        // CU2_{1,2}(theta, theta') (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
        //   = a_{00} |00> + a_{01} |01> + (a_{10} - e^{i theta'} a_{11})/sqrt(2) |10>
        //     + (e^{i theta} a_{10} + e^{i(theta + theta')} a_{11})/sqrt(2) |11>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit)
        {
          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_control_qubit = permutation[control_qubit];
          if (::ket::mpi::page::is_on_page(permutated_target_qubit, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
              return ::ket::mpi::gate::page::cphase_shift2_tcp(
                parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);

            return ::ket::mpi::gate::page::cphase_shift2_tp(
              parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);
          }
          else if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
            return ::ket::mpi::gate::page::cphase_shift2_cp(
              parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift2(
                parallel_policy, first, last,
                phase1, phase2, permutated_target_qubit.qubit(), permutated_control_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift2(
              parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // C...CU2_{tc...c'}(theta, theta') or CnU2_{tc...c'}(theta, theta')
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename Allocator,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& do_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ControlQubits const... control_qubits)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::phase_shift2(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              phase1, phase2, permutation[target_qubit].qubit(),
              permutation[control_qubit1].qubit(), permutation[control_qubit2].qubit(),
              permutation[control_qubits].qubit()...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift2(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift2(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase1, ' ', phase2, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase1, ' ', phase2, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase1, ' ', phase2),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase1, ' ', phase2),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift2(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift2(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      namespace phase_shift_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Real, typename Qubit>
        struct call_adj_phase_shift2
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_adj_phase_shift2(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::adj_phase_shift2(
              parallel_policy_, first, last, phase1_, phase2_, permutated_qubit_.qubit());
          }
        }; // struct call_adj_phase_shift2<ParallelPolicy, Real, Qubit>

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        struct call_adj_cphase_shift2
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          ::ket::mpi::permutated<TargetQubit> permutated_target_qubit_;
          ::ket::mpi::permutated<ControlQubit> permutated_control_qubit_;

          call_adj_cphase_shift2(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2,
            ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
            ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              permutated_target_qubit_{permutated_target_qubit},
              permutated_control_qubit_{permutated_control_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::adj_phase_shift2(
              parallel_policy_, first, last,
              phase1_, phase2_, permutated_target_qubit_.qubit(), permutated_control_qubit_.qubit());
          }
        }; // struct call_adj_cphase_shift2<ParallelPolicy, Real, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Real, typename Qubit>
        inline call_adj_phase_shift2<ParallelPolicy, Real, Qubit>
        make_call_adj_phase_shift2(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase1, phase2, permutated_qubit}; }

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        inline call_adj_cphase_shift2<ParallelPolicy, Real, TargetQubit, ControlQubit>
        make_call_adj_phase_shift2(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2,
          ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
          ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
        { return {parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        // U2+_i(theta, theta')
        // U2+_1(theta, theta') (a_0 |0> + a_1 |1>) = (a_0 + e^{-i theta} a_1)/sqrt(2) |0> + (-e^{-i theta'} a_0 + e^{-i(theta + theta')} a_1)/sqrt(2) |1>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_adj_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::adj_phase_shift2(
              parallel_policy, local_state, phase1, phase2, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::adj_phase_shift2(
                parallel_policy, first, last, phase1, phase2, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_adj_phase_shift2(
              parallel_policy, phase1, phase2, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // CU2+_{tc}(theta, theta') or C1U2+_{tc}(theta, theta')
        // CU2+_{1,2}(theta, theta') (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
        //   = a_{00} |00> + a_{01} |01> + (a_{10} + e^{-i theta} a_{11})/sqrt(2) |10> + (-e^{-i theta'} a_{10} + e^{-i(theta + theta')} a_{11})/sqrt(2) |11>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_adj_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit)
        {
          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_control_qubit = permutation[control_qubit];
          if (::ket::mpi::page::is_on_page(permutated_target_qubit, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
              return ::ket::mpi::gate::page::adj_cphase_shift2_tcp(
                parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);

            return ::ket::mpi::gate::page::adj_cphase_shift2_tp(
              parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);
          }
          else if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
            return ::ket::mpi::gate::page::adj_cphase_shift2_cp(
              parallel_policy, local_state, phase1, phase2, permutated_target_qubit, permutated_control_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::adj_phase_shift2(
                parallel_policy, first, last,
                phase1, phase2, permutated_target_qubit.qubit(), permutated_control_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_adj_phase_shift2(
              parallel_policy, phase1, phase2, permutated_target_qubit, permutated_control_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // C...CU2+_{tc...c'}(theta, theta') or CnU2+_{tc...c'}(theta, theta')
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename Allocator,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& do_adj_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ControlQubits const... control_qubits)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::adj_phase_shift2(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              phase1, phase2, permutation[target_qubit].qubit(),
              permutation[control_qubit1].qubit(), permutation[control_qubit2].qubit(),
              permutation[control_qubits].qubit()...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_adj_phase_shift2(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift2(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_adj_phase_shift2(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase1, ' ', phase2, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase1, ' ', phase2, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase1, ' ', phase2),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase1, ' ', phase2),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift2(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift2(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift2(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift2(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift2(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, target_qubit, control_qubits...);
      }

      namespace phase_shift_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Real, typename Qubit>
        struct call_phase_shift3
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          Real phase3_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_phase_shift3(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, Real const phase3,
            ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              phase3_{phase3},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift3(
              parallel_policy_, first, last, phase1_, phase2_, phase3_, permutated_qubit_.qubit());
          }
        }; // struct call_phase_shift3<ParallelPolicy, Real, Qubit>

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        struct call_cphase_shift3
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          Real phase3_;
          ::ket::mpi::permutated<TargetQubit> permutated_target_qubit_;
          ::ket::mpi::permutated<ControlQubit> permutated_control_qubit_;

          call_cphase_shift3(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, Real const phase3,
            ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
            ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              phase3_{phase3},
              permutated_target_qubit_{permutated_target_qubit},
              permutated_control_qubit_{permutated_control_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::phase_shift3(
              parallel_policy_, first, last,
              phase1_, phase2_, phase3_, permutated_target_qubit_.qubit(), permutated_control_qubit_.qubit());
          }
        }; // struct call_cphase_shift3<ParallelPolicy, Real, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Real, typename Qubit>
        inline call_phase_shift3<ParallelPolicy, Real, Qubit>
        make_call_phase_shift3(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, Real const phase3, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase1, phase2, phase3, permutated_qubit}; }

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        inline call_cphase_shift3<ParallelPolicy, Real, TargetQubit, ControlQubit>
        make_call_phase_shift3(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
          ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
        { return {parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        // U3_i(theta, theta', theta'')
        // U3_1(theta, theta', theta'') (a_0 |0> + a_1 |1>) = (cos(theta/2) a_0 - e^{i theta''} sin(theta/2) a_1) |0> + (e^{i theta'} sin(theta/2) a_0 + e^{i(theta' + theta'')} cos(theta/2) a_1) |1>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const qubit)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::phase_shift3(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, phase3, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift3(
                parallel_policy, first, last, phase1, phase2, phase3, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift3(
              parallel_policy, phase1, phase2, phase3, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // CU3_{tc}(theta, theta', theta'') or C1U3_{tc}(theta, theta', theta'')
        // CU3_{1,2}(theta, theta', theta'') (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
        //   = a_{00} |00> + a_{01} |01> + (cos(theta/2) a_{10} - e^{i theta''} sin(theta/2) a_{11}) |10>
        //     + (e^{i theta'} sin(theta/2) a_{10} + e^{i(theta' + theta'')} cos(theta/2) a_{11}) |11>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit)
        {
          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_control_qubit = permutation[control_qubit];
          if (::ket::mpi::page::is_on_page(permutated_target_qubit, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
              return ::ket::mpi::gate::page::cphase_shift3_tcp(
                parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);

            return ::ket::mpi::gate::page::cphase_shift3_tp(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);
          }
          else if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
            return ::ket::mpi::gate::page::cphase_shift3_cp(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::phase_shift3(
                parallel_policy, first, last,
                phase1, phase2, phase3, permutated_target_qubit.qubit(), permutated_control_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_phase_shift3(
              parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // C...CU3_{tc...c'}(theta, theta', theta'') or CnU3_{tc...c'}(theta, theta', theta'')
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename Allocator,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& do_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ControlQubits const... control_qubits)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::phase_shift3(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              phase1, phase2, phase3, permutation[target_qubit].qubit(),
              permutation[control_qubit1].qubit(), permutation[control_qubit2].qubit(),
              permutation[control_qubits].qubit()...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift3(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_phase_shift3(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase1, ' ', phase2, ' ', phase3, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Phase "}, phase1, ' ', phase2, ' ', phase3, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase1, ' ', phase2, ' ', phase3),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string(sizeof...(ControlQubits), 'C').append("Phase "), phase1, ' ', phase2, ' ', phase3),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift3(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift3(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      // U3+_i(theta, theta', theta'')
      // U3+_1(theta, theta', theta'') (a_0 |0> + a_1 |1>) = (cos(theta/2) a_0 + e^{-i theta'} sin(theta/2) a_1) |0> + (-e^{-i theta''} sin(theta/2) a_0 + e^{-i(theta' + theta'')} cos(theta/2) a_1) |1>
      namespace phase_shift_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Real, typename Qubit>
        struct call_adj_phase_shift3
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          Real phase3_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_adj_phase_shift3(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, Real const phase3,
            ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              phase3_{phase3},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::adj_phase_shift3(
              parallel_policy_, first, last, phase1_, phase2_, phase3_, permutated_qubit_.qubit());
          }
        }; // struct call_adj_phase_shift3<ParallelPolicy, Real, Qubit>

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        struct call_adj_cphase_shift3
        {
          ParallelPolicy parallel_policy_;
          Real phase1_;
          Real phase2_;
          Real phase3_;
          ::ket::mpi::permutated<TargetQubit> permutated_target_qubit_;
          ::ket::mpi::permutated<ControlQubit> permutated_control_qubit_;

          call_adj_cphase_shift3(
            ParallelPolicy const parallel_policy,
            Real const phase1, Real const phase2, Real const phase3,
            ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
            ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
            : parallel_policy_{parallel_policy},
              phase1_{phase1},
              phase2_{phase2},
              phase3_{phase3},
              permutated_target_qubit_{permutated_target_qubit},
              permutated_control_qubit_{permutated_control_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(
            RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::adj_phase_shift3(
              parallel_policy_, first, last,
              phase1_, phase2_, phase3_, permutated_target_qubit_.qubit(), permutated_control_qubit_.qubit());
          }
        }; // struct call_adj_cphase_shift3<ParallelPolicy, Real, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Real, typename Qubit>
        inline call_adj_phase_shift3<ParallelPolicy, Real, Qubit>
        make_call_adj_phase_shift3(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, Real const phase3, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase1, phase2, phase3, permutated_qubit}; }

        template <typename ParallelPolicy, typename Real, typename TargetQubit, typename ControlQubit>
        inline call_adj_cphase_shift3<ParallelPolicy, Real, TargetQubit, ControlQubit>
        make_call_adj_phase_shift3(
          ParallelPolicy const parallel_policy,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::mpi::permutated<TargetQubit> const permutated_target_qubit,
          ::ket::mpi::permutated<ControlQubit> const permutated_control_qubit)
        { return {parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_adj_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const qubit)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::adj_phase_shift3(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, phase3, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::adj_phase_shift3(
                parallel_policy, first, last, phase1, phase2, phase3, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_adj_phase_shift3(
              parallel_policy, phase1, phase2, phase3, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // CU3+_{tc}(theta, theta', theta'') or C1U3+_{tc}(theta, theta', theta'')
        // CU3+_{1,2}(theta, theta', theta'') (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
        //   = a_{00} |00> + a_{01} |01> + (cos(theta/2) a_{10} + e^{-i theta'} sin(theta/2) a_{11}) |10> + (-e^{-i theta''} sin(theta/2) a_{10} + e^{-i(theta' + theta'')} cos(theta/2) a_{11}) |11>
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename Real>
        inline RandomAccessRange& do_adj_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit)
        {
          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_control_qubit = permutation[control_qubit];
          if (::ket::mpi::page::is_on_page(permutated_target_qubit, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
              return ::ket::mpi::gate::page::adj_cphase_shift3_tcp(
                parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);

            return ::ket::mpi::gate::page::adj_cphase_shift3_tp(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);
          }
          else if (::ket::mpi::page::is_on_page(permutated_control_qubit, local_state))
            return ::ket::mpi::gate::page::adj_cphase_shift3_cp(
              parallel_policy, local_state, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::adj_phase_shift3(
                parallel_policy, first, last,
                phase1, phase2, phase3, permutated_target_qubit.qubit(), permutated_control_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::phase_shift_detail::make_call_adj_phase_shift3(
              parallel_policy, phase1, phase2, phase3, permutated_target_qubit, permutated_control_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        // C...CU3+_{tc...c'}(theta, theta', theta'') or CnU3+_{tc...c'}(theta, theta', theta'')
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename Allocator,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& do_adj_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ControlQubits const... control_qubits)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::adj_phase_shift3(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              phase1, phase2, phase3, permutation[target_qubit].qubit(),
              permutation[control_qubit1].qubit(), permutation[control_qubit2].qubit(),
              permutation[control_qubits].qubit()...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_adj_phase_shift3(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype,
          typename Real, typename... ControlQubits>
        inline RandomAccessRange& adj_phase_shift3(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment,
          Real const phase1, Real const phase2, Real const phase3,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(ControlQubits) + 1u>{target_qubit, ::ket::remove_control(control_qubits)...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::phase_shift_detail::do_adj_phase_shift3(
            mpi_policy, parallel_policy, local_state, permutation, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
        }
      } // namespace phase_shift_detail

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase1, ' ', phase2, ' ', phase3, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Phase) "}, phase1, ' ', phase2, ' ', phase3, ' ', qubit), environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase1, ' ', phase2, ' ', phase3),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(
              std::string{"Adj("}.append(sizeof...(ControlQubits), 'C').append("Phase) "), phase1, ' ', phase2, ' ', phase3),
            target_qubit, control_qubits...),
          environment};

        return ::ket::mpi::gate::phase_shift_detail::adj_phase_shift3(
          mpi_policy, parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift3(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift3(
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, qubit);
      }

      // [[deprecated]]
      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, qubit);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype,
        typename Real, typename... ControlQubits>
      inline RandomAccessRange& adj_phase_shift3(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment,
        Real const phase1, Real const phase2, Real const phase3,
        ::ket::qubit<StateInteger, BitInteger> const target_qubit, ControlQubits const... control_qubits)
      {
        return ::ket::mpi::gate::adj_phase_shift3(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, permutation, buffer, datatype, communicator, environment, phase1, phase2, phase3, target_qubit, control_qubits...);
      }
    } // namespace gate
  } // namespace mpi
} // namespace ket


#endif // KET_MPI_GATE_DETAIL_PHASE_SHIFT_STANDARD_HPP
