#ifndef KET_MPI_GATE_DETAIL_PAULI_Z_DIAGONAL_HPP
# define KET_MPI_GATE_DETAIL_PAULI_Z_DIAGONAL_HPP

# include <boost/config.hpp>

# include <complex>
# include <vector>
# include <array>
# include <tuple>
# include <iterator>

# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>

# include <ket/qubit.hpp>
# ifdef KET_PRINT_LOG
#   include <ket/qubit_io.hpp>
# endif // KET_PRINT_LOG
# include <ket/gate/pauli_z.hpp>
# include <ket/utility/meta/real_of.hpp>
# include <ket/utility/meta/index_sequence.hpp>
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
#   include <ket/mpi/permutated.hpp>
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/simple_mpi.hpp>
# include <ket/mpi/utility/for_each_local_range.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/gate/page/pauli_z.hpp>
# include <ket/mpi/page/is_on_page.hpp>


namespace ket
{
  namespace mpi
  {
    namespace gate
    {
      // Z_i
      // Z_1 (a_0 |0> + a_1 |1>) = a_0 |0> - a_1 |1>
      // ZZ_i = Z_i Z_j
      // ZZ_{1,2} (a_{00} |00> + a_{01} |01> + a_{10} |10> + a{11} |11>)
      //   = a_{00} |00> - a_{01} |01> - a_{10} |10> + a{11} |11>
      namespace pauli_z_detail
      {
# ifdef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename StateInteger>
        struct return_
        {
          template <typename Iterator>
          void operator()(Iterator const, StateInteger const) const { }
        }; // struct return_<StateInteger>

        template <typename StateInteger>
        struct negate
        {
          template <typename Iterator>
          void operator()(Iterator const iter, StateInteger const) const
          {
            using complex_type = typename std::iterator_traits<Iterator>::value_type;
            using real_type = typename ::ket::utility::meta::real_of<complex_type>::type;
            *iter *= real_type{-1};
          }
        }; // struct negate<StateInteger>
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& pauli_z(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const permutated_qubit = permutation[qubit];
          if (::ket::mpi::page::is_on_page(permutated_qubit, local_state))
            return ::ket::mpi::gate::page::pauli_z1(parallel_policy, local_state, permutated_qubit);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          using real_type = typename ::ket::utility::meta::real_of<complex_type>::type;
          ::ket::mpi::utility::diagonal_loop(
            mpi_policy, parallel_policy,
            local_state, permutation, communicator, environment, qubit,
            [](auto const, StateInteger const) { },
            [](auto const iter, StateInteger const) { *iter *= real_type{-1}; });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          ::ket::mpi::utility::diagonal_loop(
            mpi_policy, parallel_policy,
            local_state, permutation, communicator, environment, qubit,
            ::ket::mpi::gate::pauli_z_detail::return_<StateInteger>{},
            ::ket::mpi::gate::pauli_z_detail::negate<StateInteger>{});
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

          return local_state;
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& pauli_z(
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
            if (::ket::mpi::page::is_on_page(permutated_qubit1, local_state))
              return ::ket::mpi::gate::page::pauli_z2_2p(parallel_policy, local_state, permutated_qubit1, permutated_qubit2);
            else
              return ::ket::mpi::gate::page::pauli_z2_p(parallel_policy, local_state, permutated_qubit1, permutated_qubit2);
          else
            return ::ket::mpi::gate::page::pauli_z2_p(parallel_policy, local_state, permutated_qubit2, permutated_qubit1);

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
          using complex_type = typename boost::range_value<RandomAccessRange>::type;
          using real_type = typename ::ket::utility::meta::real_of<complex_type>::type;
          ::ket::mpi::utility::diagonal_loop(
            mpi_policy, parallel_policy,
            local_state, permutation, communicator, environment, qubit1, qubit2,
            [](auto const, StateInteger const) { },
            [](auto const iter, StateInteger const) { *iter *= real_type{-1}; },
            [](auto const iter, StateInteger const) { *iter *= real_type{-1}; },
            [](auto const, StateInteger const) { });
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
          ::ket::mpi::utility::diagonal_loop(
            mpi_policy, parallel_policy,
            local_state, permutation, communicator, environment, qubit1, qubit2,
            ::ket::mpi::gate::pauli_z_detail::return_<StateInteger>{},
            ::ket::mpi::gate::pauli_z_detail::negate<StateInteger>{},
            ::ket::mpi::gate::pauli_z_detail::negate<StateInteger>{},
            ::ket::mpi::gate::pauli_z_detail::return_<StateInteger>{});
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

          return local_state;
        }

# ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename IndexSequence>
        struct do_nompi_pauli_z;

        template <std::size_t... indices>
        struct do_nompi_pauli_z< ::ket::utility::meta::index_sequence<indices...> >
        {
          template <typename ParallelPolicy, typename RandomAccessIterator, typename... PermutatedQubits>
          static void call(
            ParallelPolicy const parallel_policy,
            RandomAccessIterator const first, RandomAccessIterator const last,
            std::tuple<PermutatedQubits...> const& permutated_qubits)
          { ::ket::gate::pauli_z(parallel_policy, first, last, std::get<indices>(permutated_qubits)...); }
        }; // struct do_nompi_pauli_z< ::ket::utility::meta::index_sequence<indices...> >

        template <typename ParallelPolicy, typename RandomAccessIterator, typename... PermutatedQubits>
        inline void nompi_pauli_z(
          ParallelPolicy const parallel_policy,
          RandomAccessIterator const first, RandomAccessIterator const last,
          std::tuple<PermutatedQubits...> const& permutated_qubits)
        { ::ket::mpi::gate::pauli_z_detail::do_nompi_pauli_z<typename ::ket::utility::meta::generate_index_sequence<sizeof...(PermutatedQubits)>::type>::call(parallel_policy, first, last, permutated_qubits); }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename... Qubits, typename Allocator>
        inline RandomAccessRange& do_pauli_z(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit1,
          ::ket::qubit<StateInteger, BitInteger> const qubit2,
          ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          auto const permutated_qubits = std::make_tuple(permutation[qubit1], permutation[qubit2], permutation[qubit3], permutation[qubits]...);

          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [parallel_policy, &permutated_qubits](auto const first, auto const last)
            { ::ket::mpi::gate::pauli_z_detail::nompi_pauli_z(parallel_policy, first, last, permutated_qubits); });
        }
# else // BOOST_NO_CXX14_GENERIC_LAMBDAS
        template <typename ParallelPolicy, typename IndexSequence, typename... Qubits>
        struct call_pauli_zn;

        template <typename ParallelPolicy, std::size_t... indices, typename... Qubits>
        struct call_pauli_zn<ParallelPolicy, ::ket::utility::meta::index_sequence<indices...>, Qubits...>
        {
          static_assert(sizeof...(Qubits) == sizeof...(indices), "The numbers of variadic templates should be the same");
          ParallelPolicy parallel_policy_;
          std::tuple< ::ket::mpi::permutated<Qubits>... > permutated_qubits_;

          call_pauli_zn(
            ParallelPolicy const parallel_policy,
            ::ket::mpi::permutated<Qubits> const... permutated_qubits)
            : parallel_policy_{parallel_policy},
              permutated_qubits_{permutated_qubits...}
          { }

          template <typename RandomAccessIterator>
          void operator()(RandomAccessIterator const first, RandomAccessIterator const last) const
          { ::ket::gate::pauli_z(parallel_policy_, first, last, std::get<indices>(permutated_qubits_)...); }
        }; // struct call_pauli_zn<ParallelPolicy, ::ket::utility::meta::index_sequence<indices...>, Qubits...>

        template <typename ParallelPolicy, typename Qubit, typename... Qubits>
        inline call_pauli_zn<ParallelPolicy, ::ket::utility::meta::generate_index_sequence<sizeof...(Qubits) + 3u>, Qubit, Qubit, Qubit, Qubits...> make_call_pauli_z(
          ParallelPolicy const parallel_policy,
          ::ket::mpi::permutated<Qubit> const permutated_qubit1,
          ::ket::mpi::permutated<Qubit> const permutated_qubit2,
          ::ket::mpi::permutated<Qubit> const permutated_qubit3,
          ::ket::mpi::permutated<Qubits> const... permutated_qubits)
        { return {parallel_policy, permutated_qubit1, permutated_qubit2, permutated_qubit3, permutated_qubits...}; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename... Qubits, typename Allocator>
        inline RandomAccessRange& do_pauli_z(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit1,
          ::ket::qubit<StateInteger, BitInteger> const qubit2,
          ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            ::ket::mpi::gate::pauli_z_detail::make_call_pauli_z(
              parallel_policy, permutation[qubit1], permutation[qubit2], permutation[qubit3], permutation[qubits]...));
        }
# endif // BOOST_NO_CXX14_GENERIC_LAMBDAS

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& pauli_z(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit1,
          ::ket::qubit<StateInteger, BitInteger> const qubit2,
          ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(Qubits) + 1u>{qubit1, qubit2, qubit3, qubits...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, communicator, environment);

          return ::ket::mpi::gate::pauli_z_detail::do_pauli_z(
            mpi_policy, parallel_policy, local_state, qubit1, qubit2, qubit3, qubits..., permutation, communicator, environment);
        }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator, typename BufferAllocator, typename DerivedDatatype>
        inline RandomAccessRange& pauli_z(
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
          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto qubit_array = std::array<qubit_type, sizeof...(Qubits) + 1u>{qubit1, qubit2, qubit3, qubits...};
          ::ket::mpi::utility::maybe_interchange_qubits(
            mpi_policy, parallel_policy,
            local_state, qubit_array, permutation, buffer, datatype, communicator, environment);

          return ::ket::mpi::gate::pauli_z_detail::do_pauli_z(
            mpi_policy, parallel_policy, local_state, qubit1, qubit2, qubit3, qubits..., permutation, communicator, environment);
        }
      } // namespace pauli_z_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Z "}, qubit), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy, local_state, qubit, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::datatype_base<DerivedDatatype> const&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Z "}, qubit), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy, local_state, qubit, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"ZZ "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy, local_state, qubit1, qubit2, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::datatype_base<DerivedDatatype> const&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"ZZ "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy, local_state, qubit1, qubit2, permutation, communicator, environment);
      }

      namespace pauli_z_detail
      {
        inline std::string do_generate_pauli_z_string(std::string const& result)
        { return result; }

        template <typename StateInteger, typename BitInteger, typename... Qubits>
        inline std::string do_generate_pauli_z_string(std::string const& result, ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits)
        { return do_generate_pauli_z_string(::ket::mpi::utility::generate_logger_string(result, ' ', qubit), qubits...); }

        template <typename... Qubits>
        inline std::string generate_pauli_z_string(Qubits const... qubits)
        {
          auto result = std::string{};
          for (std::size_t count = 0u; count < sizeof...(Qubits); ++count)
            result += 'Z';

          return do_generate_pauli_z_string(result, qubits...);
        }
      } // namespace pauli_z_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::gate::pauli_z_detail::generate_pauli_z_string(qubit1, qubit2, qubit3, qubits...), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_z(
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
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::gate::pauli_z_detail::generate_pauli_z_string(qubit1, qubit2, qubit3, qubits...), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_z(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_z(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& pauli_z(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& pauli_z(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      namespace pauli_z_detail
      {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
          typename Allocator>
        inline RandomAccessRange& adj_pauli_z(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::gate::pauli_z_detail::pauli_z(
            mpi_policy, parallel_policy,
            local_state, qubit, qubits..., permutation, communicator, environment);
        }
      } // namespace pauli_z_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Z) "}, qubit), environment};

        return ::ket::mpi::gate::pauli_z_detail::adj_pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::datatype_base<DerivedDatatype> const&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(Z) "}, qubit), environment};

        return ::ket::mpi::gate::pauli_z_detail::adj_pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(ZZ) "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_z_detail::adj_pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>&,
        yampi::datatype_base<DerivedDatatype> const&,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::utility::generate_logger_string(std::string{"Adj(ZZ) "}, qubit1, ' ', qubit2), environment};

        return ::ket::mpi::gate::pauli_z_detail::adj_pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, permutation, communicator, environment);
      }

      namespace pauli_z_detail
      {
        inline std::string do_generate_adj_pauli_z_string(std::string const& result)
        { return result; }

        template <typename StateInteger, typename BitInteger, typename... Qubits>
        inline std::string do_generate_adj_pauli_z_string(std::string const& result, ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits)
        { return do_generate_adj_pauli_z_string(::ket::mpi::utility::generate_logger_string(result, ' ', qubit), qubits...); }

        template <typename... Qubits>
        inline std::string generate_adj_pauli_z_string(Qubits const... qubits)
        {
          auto result = std::string{"Adj("};
          for (std::size_t count = 0u; count < sizeof...(Qubits); ++count)
            result += 'Z';
          result += ')';

          return do_generate_adj_pauli_z_string(result, qubits...);
        }
      } // namespace pauli_z_detail

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_z(
        MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit1,
        ::ket::qubit<StateInteger, BitInteger> const qubit2,
        ::ket::qubit<StateInteger, BitInteger> const qubit3, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::gate::pauli_z_detail::generate_adj_pauli_z_string(qubit1, qubit2, qubit3, qubits...), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename MpiPolicy, typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_z(
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
        ::ket::mpi::utility::log_with_time_guard<char> print{::ket::mpi::gate::pauli_z_detail::generate_adj_pauli_z_string(qubit1, qubit2, qubit3, qubits...), environment};

        return ::ket::mpi::gate::pauli_z_detail::pauli_z(
          mpi_policy, parallel_policy,
          local_state, qubit1, qubit2, qubit3, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_z(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_z(
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(),
          ::ket::utility::policy::make_sequential(),
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }

      template <
        typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& adj_pauli_z(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, communicator, environment);
      }

      template <
        typename ParallelPolicy,
        typename RandomAccessRange, typename StateInteger, typename BitInteger, typename... Qubits,
        typename Allocator, typename BufferAllocator, typename DerivedDatatype>
      inline RandomAccessRange& adj_pauli_z(
        ParallelPolicy const parallel_policy,
        RandomAccessRange& local_state,
        ::ket::qubit<StateInteger, BitInteger> const qubit, Qubits const... qubits,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::datatype_base<DerivedDatatype> const& datatype,
        yampi::communicator const& communicator, yampi::environment const& environment)
      {
        return ::ket::mpi::gate::adj_pauli_z(
          ::ket::mpi::utility::policy::make_simple_mpi(), parallel_policy,
          local_state, qubit, qubits..., permutation, buffer, datatype, communicator, environment);
      }
    } // namespace gate
  } // namespace mpi
} // namespace ket


#endif // KET_MPI_GATE_DETAIL_PAULI_Z_DIAGONAL_HPP
