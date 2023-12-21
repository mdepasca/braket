#ifndef KET_MPI_GATE_PAULI_Y_HPP
# define KET_MPI_GATE_PAULI_Y_HPP

# include <boost/config.hpp>

# include <vector>
# include <array>
# include <tuple>

# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>

# include <ket/qubit.hpp>
# ifdef KET_PRINT_LOG
#   include <ket/qubit_io.hpp>
# endif // KET_PRINT_LOG
# include <ket/gate/pauli_y.hpp>
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
#   include <ket/mpi/permutated.hpp>
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/simple_mpi.hpp>
# include <ket/mpi/utility/for_each_local_range.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/gate/detail/append_qubits_string.hpp>
# include <ket/mpi/gate/page/pauli_y.hpp>
# include <ket/mpi/page/is_on_page.hpp>


namespace ket
{
  namespace mpi
  {
    namespace gate
    {
      // Y_i
      // Y_1 (a_0 |0> + a_1 |1>) = -i a_1 |0> + i a_0 |1>
      // YY_{ij} = Y_i Y_j
      // YY_{1,2} (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
      //   = -a_{11} |00> + a_{10} |01> + a_{01} |10> - a_{00} |11>
      namespace pauli_y_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename Qubit>
        struct call_pauli_y1
        {
          ParallelPolicy parallel_policy_;
          ::ket::mpi::permutated<Qubit> permutated_qubit_;

          call_pauli_y1(ParallelPolicy const parallel_policy, ::ket::mpi::permutated<Qubit> const permutated_qubit)
            : parallel_policy_{parallel_policy},
              permutated_qubit_{permutated_qubit}
          { }

          template <typename RandomAccessIterator>
          void operator()(RandomAccessIterator const first, RandomAccessIterator const last) const
          { ::ket::gate::pauli_y(parallel_policy_, first, last, permutated_qubit_.qubit()); }
        }; // struct call_pauli_y1<ParallelPolicy, Qubit>

        template <typename ParallelPolicy, typename Qubit>
        struct call_pauli_y2
        {
          ParallelPolicy parallel_policy_;
          ::ket::mpi::permutated<Qubit> permutated_qubit1_;
          ::ket::mpi::permutated<Qubit> permutated_qubit2_;

          call_pauli_y2(
            ParallelPolicy const parallel_policy,
            ::ket::mpi::permutated<Qubit> const permutated_qubit1,
            ::ket::mpi::permutated<Qubit> const permutated_qubit2)
            : parallel_policy_{parallel_policy},
              permutated_qubit1_{permutated_qubit1},
              permutated_qubit2_{permutated_qubit2}
          { }

          template <typename RandomAccessIterator>
          void operator()(RandomAccessIterator const first, RandomAccessIterator const last) const
          { ::ket::gate::pauli_y(parallel_policy_, first, last, permutated_qubit1_.qubit(), permutated_qubit2_.qubit()); }
        }; // struct call_pauli_y2<ParallelPolicy, Qubit>

        template <typename ParallelPolicy, typename Qubit>
        inline call_pauli_y1<ParallelPolicy, Qubit> make_call_pauli_y(
          ParallelPolicy const parallel_policy, ::ket::mpi::permutated<Qubit> const permutated_qubit)
        { return {parallel_policy, permutated_qubit}; }

        template <typename ParallelPolicy, typename Qubit>
        inline call_pauli_y2<ParallelPolicy, Qubit> make_call_pauli_y(
          ParallelPolicy const parallel_policy,
          ::ket::mpi::permutated<Qubit> const permutated_qubit1,
          ::ket::mpi::permutated<Qubit> const permutated_qubit2)
        { return {parallel_policy, permutated_qubit1, permutated_qubit2}; }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::pauli_y1(parallel_policy, local_state, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, permutated_qubit](auto const first, auto const last)
            { ::ket::gate::pauli_y(parallel_policy, first, last, permutated_qubit.qubit()); });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::pauli_y_detail::make_call_pauli_y(parallel_policy, permutated_qubit));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
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
              return ::ket::mpi::gate::page::pauli_y2_2p(
                parallel_policy, local_state, permutated_qubit1, permutated_qubit2);

            return ::ket::mpi::gate::page::pauli_y2_p(
              parallel_policy, local_state, permutated_qubit1, permutated_qubit2);
          }
          else if (::ket::mpi::page::is_on_page(permutated_qubit2, local_state))
            return ::ket::mpi::gate::page::pauli_y2_p(
              parallel_policy, local_state, permutated_qubit2, permutated_qubit1);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, permutated_qubit1, permutated_qubit2](auto const first, auto const last)
            { ::ket::gate::pauli_y(parallel_policy, first, last, permutated_qubit1.qubit(), permutated_qubit2.qubit()); });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::pauli_y_detail::make_call_pauli_y(parallel_policy, permutated_qubit1, permutated_qubit2));
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename... Qubits, typename Allocator>
        inline RandomAccessRange& do_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
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
            ::ket::gate::pauli_y(
              parallel_policy,
              first + data_block_index * data_block_size,
              first + (data_block_index + 1u) * data_block_size,
              permutation[qubit1], permutation[qubit2], permutation[qubit3], permutation[qubits]...);

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
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

          return ::ket::mpi::gate::pauli_y_detail::do_pauli_y(
            mpi_policy, parallel_policy, local_state, qubit, qubits..., permutation, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
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

          return ::ket::mpi::gate::pauli_y_detail::do_pauli_y(
            mpi_policy, parallel_policy, local_state, qubit, qubits..., permutation, communicator, environment);
        }
      } // namespace pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Y "}, qubit), environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Y "}, qubit), environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"YY "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"YY "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(std::string(sizeof...(Qubits) + 3u, 'Y'), qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(std::string(sizeof...(Qubits) + 3u, 'Y'), qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::pauli_y_detail::pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_y(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_y(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      namespace pauli_y_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& adj_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::gate::pauli_y_detail::pauli_y(
            mpi_policy, parallel_policy,
            local_state, qubit, qubits..., permutation, buffer, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& adj_pauli_y(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::datatype_base<DerivedDatatype> const& datatype,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::gate::pauli_y_detail::pauli_y(
            mpi_policy, parallel_policy,
            local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
        }
      } // namespace pauli_y_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Y) "}, qubit), environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Y) "}, qubit), environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(YY) "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(YY) "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(std::string{"Adj("}.append(sizeof...(Qubits) + 3u, 'Y').append(")"), qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_y(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{
          ::ket::mpi::gate::detail::append_qubits_string(std::string{"Adj("}.append(sizeof...(Qubits) + 3u, 'Y').append(")"), qubit1, qubit2, qubit3, qubits...),
          environment};

        return ::ket::mpi::gate::pauli_y_detail::adj_pauli_y(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_y(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_y(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy, typename RandomAccessRange,
        typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_y(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_y(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }
    } // namespace gate
  } // namespace mpi
} // namespace ket

#endif // KET_MPI_GATE_PAULI_Y_HPP
