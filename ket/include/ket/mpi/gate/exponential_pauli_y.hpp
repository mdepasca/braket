#ifndef KET_MPI_GATE_EXPONENTIAL_PAULI_Y_HPP
# define KET_MPI_GATE_EXPONENTIAL_PAULI_Y_HPP

# include <boost/config.hpp>

# include <complex>
# include <vector>
# include <array>

# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>

# include <ket/qubit.hpp>
# ifdef KET_PRINT_LOG
#   include <ket/qubit_io.hpp>
# endif // KET_PRINT_LOG
# include <ket/gate/exponential_pauli_y.hpp>
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
#   include <ket/mpi/permutated.hpp>
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/simple_mpi.hpp>
# include <ket/mpi/utility/for_each_local_range.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/gate/detail/append_qubits_string.hpp>
# include <ket/mpi/gate/page/exponential_pauli_y.hpp>
# include <ket/mpi/page/is_on_page.hpp>


namespace ket
{
  namespace mpi
  {
    namespace gate
    {
      // exponential_pauli_y_coeff
      // eY_i(s) = exp(is Y_i) = I cos s + i Y_i sin s
      // eY_1(s) (a_0 |0> + a_1 |1>) = (cos s a_0 + sin s a_1) |0> + (-sin s a_0 + cos s a_1) |1>
      // eYY_{ij}(s) = exp(is Y_i Y_j) = I cos s + i Y_i Y_j sin s
      // eYY_{1,2}(s) (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
      //   = (cos s a_{00} - i sin s a_{11}) |00> + (cos s a_{01} + i sin s a_{10}) |01> + (i sin s a_{01} + cos s a_{10}) |10> + (-i sin s a_{00} + cos s a_{11}) |11>
      namespace exponential_pauli_y_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Complex, typename Qubit>
        struct call_exponential_pauli_y_coeff1
        {
          ParallelPolicy parallel_policy_;
          Complex phase_coefficient_; // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_exponential_pauli_y_coeff1(
            ParallelPolicy const parallel_policy, Complex const& phase_coefficient,
            ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              phase_coefficient_{phase_coefficient},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::exponential_pauli_y_coeff(
              parallel_policy_, first, last, phase_coefficient_, permutated_qubit_.qubit());
          }
        }; // struct call_exponential_pauli_y_coeff1<ParallelPolicy, Complex, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Complex, typename Qubit>
        struct call_exponential_pauli_y_coeff2
        {
          ParallelPolicy parallel_policy_;
          Complex phase_coefficient_; // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::mpi::permutated<Qubit> permutated_qubit1_;
          ::ket::mpi::permutated<Qubit> permutated_qubit2_;

          call_exponential_pauli_y_coeff2(
            ParallelPolicy const parallel_policy, Complex const& phase_coefficient,
            ::ket::mpi::permutated<Qubit> const permutated_qubit1,
            ::ket::mpi::permutated<Qubit> const permutated_qubit2)
            : parallel_policy_{parallel_policy},
              phase_coefficient_{phase_coefficient},
              permutated_qubit1_{permutated_qubit1},
              permutated_qubit2_{permutated_qubit2}
          { }

          template <typename RandomAccessIterator>
          void operator()(RandomAccessIterator const first, RandomAccessIterator const last) const
          {
            ::ket::gate::exponential_pauli_y_coeff(
              parallel_policy_, first, last, phase_coefficient_,
              permutated_qubit1_.qubit(), permutated_qubit2_.qubit());
          }
        }; // struct call_exponential_pauli_y_coeff2<ParallelPolicy, Complex, TargetQubit, ControlQubit>

        template <typename ParallelPolicy, typename Complex, typename Qubit>
        inline call_exponential_pauli_y_coeff1<ParallelPolicy, Complex, Qubit>
        make_call_exponential_pauli_y_coeff(
          ParallelPolicy const parallel_policy, Complex const& phase_coefficient,
          ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, phase_coefficient, permutated_qubit}; }

        template <typename ParallelPolicy, typename Complex, typename Qubit>
        inline call_exponential_pauli_y_coeff2<ParallelPolicy, Complex, Qubit>
        make_call_exponential_pauli_y_coeff(
          ParallelPolicy const parallel_policy, Complex const& phase_coefficient,
          ::ket::mpi::permutated<Qubit> const permutated_qubit1,
          ::ket::mpi::permutated<Qubit> const permutated_qubit2)
        { return {parallel_policy, phase_coefficient, permutated_qubit1, permutated_qubit2}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::exponential_pauli_y_coeff1(
              parallel_policy, local_state, phase_coefficient, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, &phase_coefficient, permutated_qubit](
              auto const first, auto const last)
            {
              ::ket::gate::exponential_pauli_y_coeff(
                parallel_policy, first, last, phase_coefficient, permutated_qubit.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::exponential_pauli_y_detail::make_call_exponential_pauli_y_coeff(
              parallel_policy, phase_coefficient, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit1,
          ::ket::qubit<StateInteger, BitInteger> const qubit2,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const permutated_qubit1 = permutation[qubit1];
          auto const permutated_qubit2 = permutation[qubit2];
          if (::ket::mpi::page::is_on_page(permutated_qubit1, local_state))
          {
            if (::ket::mpi::page::is_on_page(permutated_qubit2, local_state))
              return ::ket::mpi::gate::page::exponential_pauli_y_coeff2_2p(
                parallel_policy, local_state, phase_coefficient, permutated_qubit1, permutated_qubit2);

            return ::ket::mpi::gate::page::exponential_pauli_y_coeff2_p(
              parallel_policy, local_state, phase_coefficient, permutated_qubit1, permutated_qubit2);
          }
          else if (::ket::mpi::page::is_on_page(permutated_qubit2, local_state))
            return ::ket::mpi::gate::page::exponential_pauli_y_coeff2_p(
              parallel_policy, local_state, phase_coefficient, permutated_qubit2, permutated_qubit1);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, &phase_coefficient, permutated_qubit1, permutated_qubit2](
              auto const first, auto const last)
            {
              ::ket::gate::exponential_pauli_y_coeff(
                parallel_policy, first, last, phase_coefficient,
                permutated_qubit1.qubit(), permutated_qubit2.qubit());
            });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::exponential_pauli_y_detail::make_call_exponential_pauli_y_coeff(
              parallel_policy, phase_coefficient, permutated_qubit1, permutated_qubit2));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename... Qubits, typename Allocator>
        inline RandomAccessRange& do_exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit1,
          ::ket::qubit<StateInteger, BitInteger> const qubit2,
          ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const data_block_size
            = ::ket::mpi::utility::policy::data_block_size(mpi_policy, local_state, communicator, environment);
          auto const num_data_blocks
            = ::ket::mpi::utility::policy::num_data_blocks(mpi_policy, communicator, environment);

          auto const first = std::begin(local_state);
          for (auto data_block_index = decltype(num_data_blocks){0u}; data_block_index < num_data_blocks; ++data_block_index)
            ::ket::gate::exponential_pauli_y_coeff(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              permutation[qubit1], permutation[qubit2], permutation[qubit3], permutation[qubits]...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(Qubits) + 1u>{qubit, qubits...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::exponential_pauli_y_detail::do_exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, phase_coefficient, qubit, qubits..., permutation, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(Qubits) + 1u>{qubit, qubits...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::exponential_pauli_y_detail::do_exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, phase_coefficient, qubit, qubits..., permutation, communicator, environment);
        }
      } // namespace exponential_pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eY(coeff) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eY(coeff) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eYY(coeff) "}, phase_coefficient, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, permutation,
          buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eYY(coeff) "}, phase_coefficient, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"e"}.append(sizeof...(Qubits) + 3u, 'Y'), "(coeff) ", phase_coefficient),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"e"}.append(sizeof...(Qubits) + 3u, 'Y'), "(coeff) ", phase_coefficient),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, datatype, communicator, environment);
      }

      namespace exponential_pauli_y_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& adj_exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using std::conj;
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, conj(phase_coefficient), qubit, qubits..., permutation,
            buffer, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& adj_exponential_pauli_y_coeff(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using std::conj;
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, conj(phase_coefficient), qubit, qubits..., permutation,
            buffer, datatype, communicator, environment);
        }
      } // namespace exponential_pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eY(coeff)) "}, phase_coefficient, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit, permutation,
          buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eY(coeff)) "}, phase_coefficient, ' ', qubit1), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit, permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eYY(coeff)) "}, phase_coefficient, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, permutation,
          buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eYY(coeff)) "}, phase_coefficient, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"Adj(e"}.append(sizeof...(Qubits) + 3u, 'Y'), "(coeff)) ", phase_coefficient),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"Adj(e"}.append(sizeof...(Qubits) + 3u, 'Y'), "(coeff)) ", phase_coefficient),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y_coeff(
          mpi_policy, parallel_policy,
          local_state, phase_coefficient, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Complex,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y_coeff(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Complex const& phase_coefficient, // exp(i theta) = cos(theta) + i sin(theta)
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y_coeff(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase_coefficient, qubit, qubits..., permutation,
          buffer, datatype, communicator, environment);
      }

      // exponential_pauli_y
      namespace exponential_pauli_y_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& exponential_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const phase,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, ::ket::utility::exp_i<complex_type>(phase), qubit, qubits..., permutation,
            buffer, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& exponential_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const phase,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y_coeff(
            mpi_policy, parallel_policy,
            local_state, ::ket::utility::exp_i<complex_type>(phase), qubit, qubits..., permutation,
            buffer, datatype, communicator, environment);
        }
      } // namespace exponential_pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eY "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eY "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eYY "}, phase, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, permutation,
          buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"eYY "}, phase, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, permutation,
          buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"e"}.append(sizeof...(Qubits) + 3u, 'Y'), ' ', phase),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"e"}.append(sizeof...(Qubits) + 3u, 'Y'), ' ', phase),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y(
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y(
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& exponential_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& exponential_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      namespace exponential_pauli_y_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& adj_exponential_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const phase,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
            mpi_policy, parallel_policy,
            local_state, -phase, qubit, qubits..., permutation,
            buffer, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& adj_exponential_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const phase,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::gate::exponential_pauli_y_detail::exponential_pauli_y(
            mpi_policy, parallel_policy,
            local_state, -phase, qubit, qubits..., permutation,
            buffer, datatype, communicator, environment);
        }
      } // namespace exponential_pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eY) "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eY) "}, phase, ' ', qubit), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eYY) "}, phase, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(eYY) "}, phase, ' ', qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"Adj(e"}.append(sizeof...(Qubits) + 3u, 'Y'), ") ", phase),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(
            ::ket::mpi::utility::generate_logger_string(std::string{"Adj(e"}.append(sizeof...(Qubits) + 3u, 'Y'), ") ", phase),
            qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::exponential_pauli_y_detail::adj_exponential_pauli_y(
          mpi_policy, parallel_policy,
          local_state, phase, qubit1, qubit2, qubit3, qubits...,
          permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y(
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y(
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, phase, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_exponential_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange, typename Real,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_exponential_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        Real const phase,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_exponential_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, phase, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }
    } // namespace gate
  } // namespace mpi
} // namespace ket


#endif // KET_MPI_GATE_EXPONENTIAL_PAULI_Y_HPP
