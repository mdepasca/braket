#ifndef KET_MPI_GATE_PAGE_TOFFOLI_HPP
# define KET_MPI_GATE_PAGE_TOFFOLI_HPP

# include <cassert>
# include <array>
# include <algorithm>

# include <boost/range/size.hpp>

# include <ket/qubit.hpp>
# include <ket/control.hpp>
# include <ket/utility/loop_n.hpp>
# include <ket/utility/integer_exp2.hpp>
# include <ket/utility/begin.hpp>
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/state.hpp>


namespace ket
{
  namespace mpi
  {
    namespace gate
    {
      namespace page
      {
        // tccp: all of target qubit and two control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& toffoli_tccp(
          MpiPolicy const, ParallelPolicy const,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 0, StateAllocator>& toffoli_tccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 0, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 1, StateAllocator>& toffoli_tccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 1, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 2, StateAllocator>& toffoli_tccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 2, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, int num_page_qubits_, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>&
        toffoli_tccp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          static_assert(num_page_qubits_ >= 3, "num_page_qubits_ should be greater than or equal to 3");

          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_cqubit1 = permutation[control_qubit1.qubit()];
          auto const permutated_cqubit2 = permutation[control_qubit2.qubit()];
          assert(local_state.is_page_qubit(permutated_target_qubit));
          assert(local_state.is_page_qubit(permutated_cqubit1));
          assert(local_state.is_page_qubit(permutated_cqubit2));

          auto const num_nonpage_qubits
            = static_cast<BitInteger>(local_state.num_local_qubits() - num_page_qubits_);

          using qubit_type = ::ket::qubit<StateInteger, BitInteger>;
          auto sorted_permutated_qubits
            = std::array<qubit_type, 3u>{permutated_target_qubit, permutated_cqubit1, permutated_cqubit2};
          std::sort(::ket::utility::begin(sorted_permutated_qubits), ::ket::utility::end(sorted_permutated_qubits));

          auto const target_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutated_target_qubit - static_cast<BitInteger>(num_nonpage_qubits));
          auto const control_qubits_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutated_cqubit1 - static_cast<BitInteger>(num_nonpage_qubits))
              bitor ::ket::utility::integer_exp2<StateInteger>(
                      permutated_cqubit2 - static_cast<BitInteger>(num_nonpage_qubits));

          auto bits_mask = std::array<StateInteger, 4u>{};
          bits_mask[0u]
            = ::ket::utility::integer_exp2<StateInteger>(
                sorted_permutated_qubits[0u] - static_cast<BitInteger>(num_nonpage_qubits))
              - StateInteger{1u};
          bits_mask[1u]
            = (::ket::utility::integer_exp2<StateInteger>(
                 sorted_permutated_qubits[1u] - (BitInteger{1u} + num_nonpage_qubits)) - StateInteger{1u})
              xor bits_mask[0u];
          bits_mask[2u]
            = (::ket::utility::integer_exp2<StateInteger>(
                 sorted_permutated_qubits[2u] - (BitInteger{2u} + num_nonpage_qubits)) - StateInteger{1u})
              xor (bits_mask[0u] bitor bits_mask[1u]);
          bits_mask[3u] = compl (bits_mask[0u] bitor bits_mask[1u] bitor bits_mask[2u]);

          static constexpr auto num_pages
            = ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>::num_pages;
          for (auto page_id_wo_qubits = std::size_t{0u};
               page_id_wo_qubits < num_pages / 8u; ++page_id_wo_qubits)
          {
            // x0_cx0_tx0_cx
            auto const base_page_id
              = ((page_id_wo_qubits bitand bits_mask[3u]) << 3u)
                bitor ((page_id_wo_qubits bitand bits_mask[2u]) << 2u)
                bitor ((page_id_wo_qubits bitand bits_mask[1u]) << 1u)
                bitor (page_id_wo_qubits bitand bits_mask[0u]);
            // x1_cx0_tx1_cx
            auto const control_on_page_id = base_page_id bitor control_qubits_mask;
            // x1_cx1_tx1_cx
            auto const target_control_on_page_id = control_on_page_id bitor target_qubit_mask;

            local_state.swap_pages(control_on_page_id, target_control_on_page_id);
          }

          return local_state;
        }

        // tcp: target qubit and one of control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& toffoli_tcp(
          MpiPolicy const, ParallelPolicy const,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 0, StateAllocator>& toffoli_tcp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 0, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 1, StateAllocator>& toffoli_tcp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 1, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 2, StateAllocator>& toffoli_tcp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 2, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, int num_page_qubits_, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>&
        toffoli_tcp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& page_control_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& nonpage_control_qubit,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          static_assert(num_page_qubits_ >= 3, "num_page_qubits_ should be greater than or equal to 3");

          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_page_cqubit = permutation[page_control_qubit.qubit()];
          assert(local_state.is_page_qubit(permutated_target_qubit));
          assert(local_state.is_page_qubit(permutated_page_cqubit));
          assert(not local_state.is_page_qubit(permutation[nonpage_control_qubit.qubit()]));

          auto const num_nonpage_qubits
            = static_cast<BitInteger>(local_state.num_local_qubits() - num_page_qubits_);

          auto const minmax_page_permutated_qubits
            = std::minmax(permutated_target_qubit, permutated_page_cqubit);

          auto const target_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutated_target_qubit - static_cast<BitInteger>(num_nonpage_qubits));
          auto const page_control_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutated_page_cqubit - static_cast<BitInteger>(num_nonpage_qubits));
          auto const nonpage_control_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(permutation[nonpage_control_qubit.qubit()]);

          auto const page_lower_bits_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                minmax_page_permutated_qubits.first - static_cast<BitInteger>(num_nonpage_qubits))
              - StateInteger{1u};
          auto const page_middle_bits_mask
            = (::ket::utility::integer_exp2<StateInteger>(
                 minmax_page_permutated_qubits.second - (BitInteger{1u} + num_nonpage_qubits)) - StateInteger{1u})
              xor page_lower_bits_mask;
          auto const page_upper_bits_mask = compl (page_lower_bits_mask bitor page_middle_bits_mask);
          auto const nonpage_lower_bits_mask = nonpage_control_qubit_mask - StateInteger{1u};
          auto const nonpage_upper_bits_mask = compl nonpage_lower_bits_mask;

          static constexpr auto num_pages
            = ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>::num_pages;
          for (auto page_id_wo_qubits = std::size_t{0u};
               page_id_wo_qubits < num_pages / 4u; ++page_id_wo_qubits)
          {
            // x0_tx0_cx
            auto const base_page_id
              = ((page_id_wo_qubits bitand page_upper_bits_mask) << 2u)
                bitor ((page_id_wo_qubits bitand page_middle_bits_mask) << 1u)
                bitor (page_id_wo_qubits bitand page_lower_bits_mask);
            // x0_tx1_cx
            auto const control_on_page_id = base_page_id bitor page_control_qubit_mask;
            // x1_tx1_cx
            auto const target_control_on_page_id = control_on_page_id bitor target_qubit_mask;

            auto zero_page_range = local_state.page_range(control_on_page_id);
            auto one_page_range = local_state.page_range(target_control_on_page_id);

            auto const zero_first = ::ket::utility::begin(zero_page_range);
            auto const one_first = ::ket::utility::begin(one_page_range);

            using ::ket::utility::loop_n;
            loop_n(
              parallel_policy,
              boost::size(zero_page_range) / 2u,
              [zero_first, one_first,
               nonpage_control_qubit_mask, nonpage_lower_bits_mask, nonpage_upper_bits_mask](
                StateInteger const index_wo_qubit, int const)
              {
                auto const zero_index
                  = ((index_wo_qubit bitand nonpage_upper_bits_mask) << 1u)
                    bitor (index_wo_qubit bitand nonpage_lower_bits_mask);
                auto const one_index = zero_index bitor nonpage_control_qubit_mask;
                std::iter_swap(zero_first + one_index, one_first + one_index);
              });
          }

          return local_state;
        }

        // ccp: two control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& toffoli_ccp(
          MpiPolicy const, ParallelPolicy const,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 0, StateAllocator>& toffoli_ccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 0, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 1, StateAllocator>& toffoli_ccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 1, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 2, StateAllocator>& toffoli_ccp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 2, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, int num_page_qubits_, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>&
        toffoli_ccp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          static_assert(num_page_qubits_ >= 3, "num_page_qubits_ should be greater than or equal to 3");

          auto const permutated_cqubit1 = permutation[control_qubit1.qubit()];
          auto const permutated_cqubit2 = permutation[control_qubit2.qubit()];
          assert(not local_state.is_page_qubit(permutation[target_qubit]));
          assert(local_state.is_page_qubit(permutated_cqubit1));
          assert(local_state.is_page_qubit(permutated_cqubit2));

          auto const num_nonpage_qubits
            = static_cast<BitInteger>(local_state.num_local_qubits() - num_page_qubits_);

          auto const minmax_page_permutated_qubits
            = std::minmax(permutated_cqubit1, permutated_cqubit2);

          auto const page_control_qubits_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutated_cqubit1 - static_cast<BitInteger>(num_nonpage_qubits))
              bitor ::ket::utility::integer_exp2<StateInteger>(
                      permutated_cqubit2 - static_cast<BitInteger>(num_nonpage_qubits));
          auto const nonpage_target_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(permutation[target_qubit]);

          auto const page_lower_bits_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                minmax_page_permutated_qubits.first - static_cast<BitInteger>(num_nonpage_qubits))
              - StateInteger{1u};
          auto const page_middle_bits_mask
            = (::ket::utility::integer_exp2<StateInteger>(
                 minmax_page_permutated_qubits.second - (BitInteger{1u} + num_nonpage_qubits)) - StateInteger{1u})
              xor page_lower_bits_mask;
          auto const page_upper_bits_mask = compl (page_lower_bits_mask bitor page_middle_bits_mask);
          auto const nonpage_lower_bits_mask = nonpage_target_qubit_mask - StateInteger{1u};
          auto const nonpage_upper_bits_mask = compl nonpage_lower_bits_mask;

          static constexpr auto num_pages
            = ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>::num_pages;
          for (auto page_id_wo_qubits = std::size_t{0u};
               page_id_wo_qubits < num_pages / 4u; ++page_id_wo_qubits)
          {
            // x0_cx0_cx
            auto const base_page_id
              = ((page_id_wo_qubits bitand page_upper_bits_mask) << 2u)
                bitor ((page_id_wo_qubits bitand page_middle_bits_mask) << 1u)
                bitor (page_id_wo_qubits bitand page_lower_bits_mask);
            // x1_cx1_cx
            auto const control_on_page_id
              = base_page_id bitor page_control_qubits_mask;

            auto one_page_range = local_state.page_range(control_on_page_id);
            auto const one_first = ::ket::utility::begin(one_page_range);

            using ::ket::utility::loop_n;
            loop_n(
              parallel_policy,
              boost::size(one_page_range) / 2u,
              [one_first,
               nonpage_target_qubit_mask, nonpage_lower_bits_mask, nonpage_upper_bits_mask](
                StateInteger const index_wo_qubit, int const)
              {
                auto const zero_index
                  = ((index_wo_qubit bitand nonpage_upper_bits_mask) << 1u)
                    bitor (index_wo_qubit bitand nonpage_lower_bits_mask);
                auto const one_index = zero_index bitor nonpage_target_qubit_mask;
                std::iter_swap(one_first + zero_index, one_first + one_index);
              });
          }

          return local_state;
        }

        // tp: only target qubit is on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& toffoli_tp(
          MpiPolicy const, ParallelPolicy const,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 0, StateAllocator>& toffoli_tp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 0, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 1, StateAllocator>& toffoli_tp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 1, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 2, StateAllocator>& toffoli_tp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 2, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, int num_page_qubits_, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>&
        toffoli_tp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          static_assert(num_page_qubits_ >= 3, "num_page_qubits_ should be greater than or equal to 3");

          auto const permutated_cqubit1 = permutation[control_qubit1.qubit()];
          auto const permutated_cqubit2 = permutation[control_qubit2.qubit()];
          assert(local_state.is_page_qubit(permutation[target_qubit]));
          assert(not local_state.is_page_qubit(permutated_cqubit1));
          assert(not local_state.is_page_qubit(permutated_cqubit2));

          auto const num_nonpage_qubits
            = static_cast<BitInteger>(local_state.num_local_qubits() - num_page_qubits_);

          auto const minmax_nonpage_permutated_qubits = std::minmax(permutated_cqubit1, permutated_cqubit2);

          auto const target_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutation[target_qubit] - static_cast<BitInteger>(num_nonpage_qubits));
          auto const control_qubits_mask
            = ::ket::utility::integer_exp2<StateInteger>(permutated_cqubit1)
              bitor ::ket::utility::integer_exp2<StateInteger>(permutated_cqubit2);

          auto const page_lower_bits_mask = target_qubit_mask - StateInteger{1u};
          auto const page_upper_bits_mask = compl page_lower_bits_mask;
          auto const nonpage_lower_bits_mask
            = ::ket::utility::integer_exp2<StateInteger>(minmax_nonpage_permutated_qubits.first)
              - StateInteger{1u};
          auto const nonpage_middle_bits_mask
            = (::ket::utility::integer_exp2<StateInteger>(minmax_nonpage_permutated_qubits.second - BitInteger{1u})
               - StateInteger{1u})
              xor nonpage_lower_bits_mask;
          auto const nonpage_upper_bits_mask
            = compl (nonpage_lower_bits_mask bitor nonpage_middle_bits_mask);

          static constexpr auto num_pages
            = ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>::num_pages;
          for (auto page_id_wo_qubits = std::size_t{0u};
               page_id_wo_qubits < num_pages / 2u; ++page_id_wo_qubits)
          {
            // x0_tx
            auto const base_page_id
              = ((page_id_wo_qubits bitand page_upper_bits_mask) << 1u)
                bitor (page_id_wo_qubits bitand page_lower_bits_mask);
            // x1_tx
            auto const target_on_page_id = base_page_id bitor target_qubit_mask;

            auto zero_page_range = local_state.page_range(base_page_id);
            auto one_page_range = local_state.page_range(target_on_page_id);

            auto const zero_first = ::ket::utility::begin(zero_page_range);
            auto const one_first = ::ket::utility::begin(one_page_range);

            using ::ket::utility::loop_n;
            loop_n(
              parallel_policy,
              boost::size(zero_page_range) / 4u,
              [zero_first, one_first,
               control_qubits_mask, nonpage_lower_bits_mask, nonpage_middle_bits_mask, nonpage_upper_bits_mask](
                StateInteger const index_wo_qubit, int const)
              {
                auto const zero_index
                  = ((index_wo_qubit bitand nonpage_upper_bits_mask) << 2u)
                    bitor ((index_wo_qubit bitand nonpage_middle_bits_mask) << 1u)
                    bitor (index_wo_qubit bitand nonpage_lower_bits_mask);
                auto const one_index = zero_index bitor control_qubits_mask;
                std::iter_swap(zero_first + one_index, one_first + one_index);
              });
          }

          return local_state;
        }

        // cp: only one of control qubits is on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& toffoli_cp(
          MpiPolicy const, ParallelPolicy const,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 0, StateAllocator>& toffoli_cp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 0, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 1, StateAllocator>& toffoli_cp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 1, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline ::ket::mpi::state<Complex, 2, StateAllocator>& toffoli_cp(
          MpiPolicy const, ParallelPolicy const,
          ::ket::mpi::state<Complex, 2, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const&,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, Allocator> const&)
        { return local_state; }

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex, int num_page_qubits_, typename StateAllocator,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>&
        toffoli_cp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& page_control_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& nonpage_control_qubit,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          static_assert(num_page_qubits_ >= 3, "num_page_qubits_ should be greater than or equal to 3");

          auto const permutated_target_qubit = permutation[target_qubit];
          auto const permutated_nonpage_cqubit = permutation[nonpage_control_qubit.qubit()];
          assert(not local_state.is_page_qubit(permutated_target_qubit));
          assert(local_state.is_page_qubit(permutation[page_control_qubit.qubit()]));
          assert(not local_state.is_page_qubit(permutated_nonpage_cqubit));

          auto const num_nonpage_qubits
            = static_cast<BitInteger>(local_state.num_local_qubits() - num_page_qubits_);

          auto const minmax_nonpage_permutated_qubits
            = std::minmax(permutated_target_qubit, permutated_nonpage_cqubit);

          auto const target_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(permutated_target_qubit);
          auto const page_control_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(
                permutation[page_control_qubit.qubit()] - static_cast<BitInteger>(num_nonpage_qubits));
          auto const nonpage_control_qubit_mask
            = ::ket::utility::integer_exp2<StateInteger>(permutated_nonpage_cqubit);

          auto const page_lower_bits_mask = page_control_qubit_mask - StateInteger{1u};
          auto const page_upper_bits_mask = compl page_lower_bits_mask;
          auto const nonpage_lower_bits_mask
            = ::ket::utility::integer_exp2<StateInteger>(minmax_nonpage_permutated_qubits.first) - StateInteger{1u};
          auto const nonpage_middle_bits_mask
            = (::ket::utility::integer_exp2<StateInteger>(minmax_nonpage_permutated_qubits.second - BitInteger{1u})
               - StateInteger{1u})
              xor nonpage_lower_bits_mask;
          auto const nonpage_upper_bits_mask
            = compl (nonpage_lower_bits_mask bitor nonpage_middle_bits_mask);

          static constexpr auto num_pages
            = ::ket::mpi::state<Complex, num_page_qubits_, StateAllocator>::num_pages;
          for (auto page_id_wo_qubits = std::size_t{0u};
               page_id_wo_qubits < num_pages / 2u; ++page_id_wo_qubits)
          {
            // x0_cx
            auto const base_page_id
              = ((page_id_wo_qubits bitand page_upper_bits_mask) << 1u)
                bitor (page_id_wo_qubits bitand page_lower_bits_mask);
            // x1_cx
            auto const control_on_page_id = base_page_id bitor page_control_qubit_mask;

            auto one_page_range = local_state.page_range(control_on_page_id);
            auto const one_first = ::ket::utility::begin(one_page_range);

            using ::ket::utility::loop_n;
            loop_n(
              parallel_policy,
              boost::size(one_page_range) / 4u,
              [one_first,
               target_qubit_mask, nonpage_control_qubit_mask,
               nonpage_lower_bits_mask, nonpage_middle_bits_mask, nonpage_upper_bits_mask](
                StateInteger const index_wo_qubit, int const)
              {
                auto const base_index
                  = ((index_wo_qubit bitand nonpage_upper_bits_mask) << 2u)
                    bitor ((index_wo_qubit bitand nonpage_middle_bits_mask) << 1u)
                    bitor (index_wo_qubit bitand nonpage_lower_bits_mask);
                auto const zero_index = base_index bitor nonpage_control_qubit_mask;
                auto const one_index = zero_index bitor target_qubit_mask;
                std::iter_swap(one_first + zero_index, one_first + one_index);
              });
          }

          return local_state;
        }

        // tccp: all of target qubit and two control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline RandomAccessRange&
        adj_toffoli_tccp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          return ::ket::mpi::gate::page::toffoli_tccp(
            mpi_policy, parallel_policy, local_state,
            target_qubit, control_qubit1, control_qubit2, permutation);
        }

        // tcp: target qubit and one of two control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline RandomAccessRange&
        adj_toffoli_tcp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& page_control_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& nonpage_control_qubit,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          return ::ket::mpi::gate::page::toffoli_tcp(
            mpi_policy, parallel_policy, local_state,
            target_qubit, page_control_qubit, nonpage_control_qubit, permutation);
        }

        // ccp: two control qubits are on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline RandomAccessRange&
        adj_toffoli_ccp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          return ::ket::mpi::gate::page::toffoli_ccp(
            mpi_policy, parallel_policy, local_state,
            target_qubit, control_qubit1, control_qubit2, permutation);
        }

        // tp: only target qubit is on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline RandomAccessRange&
        adj_toffoli_tp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit1,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const control_qubit2,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          return ::ket::mpi::gate::page::toffoli_tp(
            mpi_policy, parallel_policy, local_state,
            target_qubit, control_qubit1, control_qubit2, permutation);
        }

        // cp: only one of control qubit is on page
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange,
          typename StateInteger, typename BitInteger, typename PermutationAllocator>
        inline RandomAccessRange&
        adj_toffoli_cp(
          MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          ::ket::qubit<StateInteger, BitInteger> const target_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& page_control_qubit,
          ::ket::control< ::ket::qubit<StateInteger, BitInteger> > const& nonpage_control_qubit,
          ::ket::mpi::qubit_permutation<
            StateInteger, BitInteger, PermutationAllocator> const& permutation)
        {
          return ::ket::mpi::gate::page::toffoli_cp(
            mpi_policy, parallel_policy, local_state,
            target_qubit, page_control_qubit, nonpage_control_qubit, permutation);
        }
      } // namespace page
    } // namespace gate
  } // namespace mpi
} // namespace ket


#endif // KET_MPI_GATE_PAGE_TOFFOLI_HPP
