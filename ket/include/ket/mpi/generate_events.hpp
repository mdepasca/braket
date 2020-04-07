#ifndef KET_MPI_GENERATE_EVENTS_HPP
# define KET_MPI_GENERATE_EVENTS_HPP

# include <boost/config.hpp>

# include <cmath>
# include <vector>

# include <boost/range/size.hpp>
# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>
# include <yampi/rank.hpp>
# include <yampi/buffer.hpp>
# include <yampi/gather.hpp>
# include <yampi/broadcast.hpp>
# include <yampi/message_envelope.hpp>
# include <yampi/algorithm/transform.hpp>

# include <ket/utility/loop_n.hpp>
# include <ket/utility/positive_random_value_upto.hpp>
# include <ket/utility/begin.hpp>
# include <ket/utility/meta/real_of.hpp>
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/general_mpi.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/utility/fill.hpp>
# include <ket/mpi/utility/transform_inclusive_scan.hpp>
# include <ket/mpi/utility/transform_inclusive_scan_self.hpp>
# include <ket/mpi/utility/upper_bound.hpp>


namespace ket
{
  namespace mpi
  {
    // generate_events
    namespace generate_events_detail
    {
# ifdef BOOST_NO_CXX11_LAMBDAS
      template <typename Complex>
      struct real_part_plus
      {
        typedef Complex result_type;

        Complex operator()(Complex const& lhs, Complex const& rhs) const
        { using std::real; return static_cast<Complex>(real(lhs) + real(rhs)); }
      };

      template <typename Complex>
      struct complex_norm
      {
        typedef Complex result_type;

        Complex operator()(Complex const& value) const
        { using std::norm; return static_cast<Complex>(norm(value)); }
      };

      template <typename Complex>
      struct real_part_less_than
      {
        typedef bool result_type;

        bool operator()(Complex const& lhs, Complex const& rhs) const
        { using std::real; return real(lhs) < real(rhs); }
      };


      template <typename TotalProbabilities>
      struct modify_random_value
      {
       private:
        TotalProbabilities const& total_probabilities_;
        yampi::rank result_rank_;

       public:
        typedef typename TotalProbabilities::value_type result_type;

        modify_random_value(
          TotalProbabilities const& total_probabilities, yampi::rank const result_rank)
          : total_probabilities_(total_probabilities), result_rank_(result_rank)
        { }

        result_type operator()(result_type const random_value) const
        { return random_value - total_probabilities_[result_rank_.mpi_rank()-1]; }
      };

      template <typename TotalProbabilities>
      inline ::ket::mpi::generate_events_detail::modify_random_value<TotalProbabilities>
      make_modify_random_value(
        TotalProbabilities const& total_probabilities, yampi::rank const result_rank)
      {
        return ::ket::mpi::generate_events_detail::modify_random_value<TotalProbabilities>(
          total_probabilities, result_rank);
      }
# endif // BOOST_NO_CXX11_LAMBDAS
    }

    template <
      typename MpiPolicy, typename ParallelPolicy,
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator>
    inline void generate_events(
      MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator& random_number_generator,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      ket::mpi::utility::log_with_time_guard<char> print("Generate Events", environment);

      result.clear();
      result.reserve(num_events);

      typedef typename boost::range_value<LocalState>::type complex_type;
      typedef typename ::ket::utility::meta::real_of<complex_type>::type real_type;
      using std::real;
# ifndef BOOST_NO_CXX11_LAMBDAS
      real_type const total_probability
        = real(::ket::mpi::utility::transform_inclusive_scan_self(
            parallel_policy, local_state,
            [](complex_type const& lhs, complex_type const& rhs)
            { using std::real; return static_cast<complex_type>(real(lhs) + real(rhs)); },
            [](complex_type const& value)
            { using std::norm; return static_cast<complex_type>(norm(value)); }));
# else // BOOST_NO_CXX11_LAMBDAS
      real_type const total_probability
        = real(::ket::mpi::utility::transform_inclusive_scan_self(
            parallel_policy, local_state,
            ::ket::mpi::generate_events_detail::real_part_plus<complex_type>(),
            ::ket::mpi::generate_events_detail::complex_norm<complex_type>()));
# endif // BOOST_NO_CXX11_LAMBDAS

      yampi::rank const present_rank = communicator.rank(environment);
      BOOST_CONSTEXPR_OR_CONST yampi::rank root_rank(0);

      std::vector<real_type> total_probabilities;
      if (present_rank == root_rank)
        total_probabilities.resize(communicator.size(environment));

      yampi::gather(root_rank, communicator).call(
        yampi::make_buffer(total_probability),
        ::ket::utility::begin(total_probabilities),
        environment);

      if (present_rank == root_rank)
        ::ket::utility::ranges::inclusive_scan(
          total_probabilities, ::ket::utility::begin(total_probabilities));

      for (int event_index = 0; event_index < num_events; ++event_index)
      {
        real_type random_value;
        yampi::rank result_rank;
        if (present_rank == root_rank)
        {
          random_value
            = ::ket::utility::positive_random_value_upto(
                total_probabilities.back(), random_number_generator);
          result_rank
            = static_cast<yampi::rank>(static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(total_probabilities, random_value)));
        }

        int result_mpi_rank = result_rank.mpi_rank();
        yampi::broadcast(root_rank, communicator).call(
          yampi::make_buffer(result_mpi_rank),
          environment);
        result_rank = static_cast<yampi::rank>(result_mpi_rank);

# ifndef BOOST_NO_CXX11_LAMBDAS
        yampi::algorithm::transform(
          yampi::ignore_status(),
          yampi::make_buffer(random_value),
          yampi::make_buffer(random_value),
          [&total_probabilities, result_rank](real_type const random_value)
          { return random_value - total_probabilities[result_rank.mpi_rank()-1]; },
          ::yampi::message_envelope(root_rank, result_rank, communicator),
          environment);
# else
        yampi::algorithm::transform(
          yampi::ignore_status(),
          yampi::make_buffer(random_value),
          yampi::make_buffer(random_value),
          ::ket::mpi::generate_events_detail::make_modify_random_value(total_probabilities, result_rank),
          ::yampi::message_envelope(root_rank, result_rank, communicator),
          environment);
# endif

        StateInteger permutated_result;
        if (present_rank == result_rank)
        {
# ifndef BOOST_NO_CXX11_LAMBDAS
          StateInteger const local_result
            = static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(
                  local_state, static_cast<complex_type>(random_value),
                  [](complex_type const& lhs, complex_type const& rhs)
                  { using std::real; return real(lhs) < real(rhs); }));
# else // BOOST_NO_CXX11_LAMBDAS
          StateInteger const local_result
            = static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(
                  local_state, static_cast<complex_type>(random_value),
                  ::ket::mpi::generate_events_detail::real_part_less_than<complex_type>()));
# endif // BOOST_NO_CXX11_LAMBDAS
          using ::ket::mpi::utility::rank_index_to_qubit_value;
          permutated_result
            = rank_index_to_qubit_value(
                mpi_policy, local_state, result_rank, local_result);
        }

        yampi::broadcast(result_rank, communicator).call(
          yampi::make_buffer(permutated_result), environment);

        using ::ket::mpi::inverse_permutate_bits;
        result.push_back(inverse_permutate_bits(permutation, permutated_result));
      }
    }

    template <
      typename MpiPolicy, typename ParallelPolicy,
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator,
      typename DerivedDatatype1, typename DerivedDatatype2>
    inline void generate_events(
      MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator& random_number_generator,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::datatype_base<DerivedDatatype1> const& state_integer_datatype,
      yampi::datatype_base<DerivedDatatype2> const& real_datatype,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      ket::mpi::utility::log_with_time_guard<char> print("Generate Events", environment);

      result.clear();
      result.reserve(num_events);

      typedef typename boost::range_value<LocalState>::type complex_type;
      typedef typename ::ket::utility::meta::real_of<complex_type>::type real_type;
      using std::real;
# ifndef BOOST_NO_CXX11_LAMBDAS
      real_type const total_probability
        = real(::ket::mpi::utility::transform_inclusive_scan_self(
            parallel_policy, local_state,
            [](complex_type const& lhs, complex_type const& rhs)
            { using std::real; return static_cast<complex_type>(real(lhs) + real(rhs)); },
            [](complex_type const& value)
            { using std::norm; return static_cast<complex_type>(norm(value)); }));
# else // BOOST_NO_CXX11_LAMBDAS
      real_type const total_probability
        = real(::ket::mpi::utility::transform_inclusive_scan_self(
            parallel_policy, local_state,
            ::ket::mpi::generate_events_detail::real_part_plus<complex_type>(),
            ::ket::mpi::generate_events_detail::complex_norm<complex_type>()));
# endif // BOOST_NO_CXX11_LAMBDAS

      yampi::rank const present_rank = communicator.rank(environment);
      BOOST_CONSTEXPR_OR_CONST yampi::rank root_rank(0);

      std::vector<real_type> total_probabilities;
      if (present_rank == root_rank)
        total_probabilities.resize(communicator.size(environment));

      yampi::gather(root_rank, communicator).call(
        yampi::make_buffer(total_probability, real_datatype),
        ::ket::utility::begin(total_probabilities),
        environment);

      if (present_rank == root_rank)
        ::ket::utility::ranges::inclusive_scan(
          total_probabilities, ::ket::utility::begin(total_probabilities));

      for (int event_index = 0; event_index < num_events; ++event_index)
      {
        real_type random_value;
        yampi::rank result_rank;
        if (present_rank == root_rank)
        {
          random_value
            = ::ket::utility::positive_random_value_upto(
                total_probabilities.back(), random_number_generator);
          result_rank
            = static_cast<yampi::rank>(static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(total_probabilities, random_value)));
        }

        int result_mpi_rank = result_rank.mpi_rank();
        yampi::broadcast(root_rank, communicator).call(
          yampi::make_buffer(result_mpi_rank),
          environment);
        result_rank = static_cast<yampi::rank>(result_mpi_rank);

# ifndef BOOST_NO_CXX11_LAMBDAS
        yampi::algorithm::transform(
          yampi::ignore_status(),
          yampi::make_buffer(random_value, real_datatype),
          yampi::make_buffer(random_value, real_datatype),
          [&total_probabilities, result_rank](real_type const random_value)
          { return random_value - total_probabilities[result_rank.mpi_rank()-1]; },
          ::yampi::message_envelope(root_rank, result_rank, communicator),
          environment);
# else
        yampi::algorithm::transform(
          yampi::ignore_status(),
          yampi::make_buffer(random_value, real_datatype),
          yampi::make_buffer(random_value, real_datatype),
          ::ket::mpi::generate_events_detail::make_modify_random_value(total_probabilities, result_rank),
          ::yampi::message_envelope(root_rank, result_rank, communicator),
          environment);
# endif

        StateInteger permutated_result;
        if (present_rank == result_rank)
        {
# ifndef BOOST_NO_CXX11_LAMBDAS
          StateInteger const local_result
            = static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(
                  local_state, static_cast<complex_type>(random_value),
                  [](complex_type const& lhs, complex_type const& rhs)
                  { using std::real; return real(lhs) < real(rhs); }));
# else // BOOST_NO_CXX11_LAMBDAS
          StateInteger const local_result
            = static_cast<StateInteger>(
                ::ket::mpi::utility::upper_bound(
                  local_state, static_cast<complex_type>(random_value),
                  ::ket::mpi::generate_events_detail::real_part_less_than<complex_type>()));
# endif // BOOST_NO_CXX11_LAMBDAS
          using ::ket::mpi::utility::rank_index_to_qubit_value;
          permutated_result
            = rank_index_to_qubit_value(
                mpi_policy, local_state, result_rank, local_result);
        }

        yampi::broadcast(result_rank, communicator).call(
          yampi::make_buffer(permutated_result, state_integer_datatype), environment);

        using ::ket::mpi::inverse_permutate_bits;
        result.push_back(inverse_permutate_bits(permutation, permutated_result));
      }
    }

    template <
      typename MpiPolicy, typename ParallelPolicy,
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator>
    inline void generate_events(
      MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator const&,
      typename RandomNumberGenerator::result_type const seed,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      RandomNumberGenerator random_number_generator(seed);
      ::ket::mpi::generate_events(
        mpi_policy, parallel_policy,
        result, local_state, num_events, random_number_generator, permutation,
        communicator, environment);
    }

    template <
      typename MpiPolicy, typename ParallelPolicy,
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator,
      typename DerivedDatatype1, typename DerivedDatatype2>
    inline void generate_events(
      MpiPolicy const mpi_policy, ParallelPolicy const parallel_policy,
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator const&,
      typename RandomNumberGenerator::result_type const seed,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::datatype_base<DerivedDatatype1> const& state_integer_datatype,
      yampi::datatype_base<DerivedDatatype2> const& real_datatype,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      RandomNumberGenerator random_number_generator(seed);
      ::ket::mpi::generate_events(
        mpi_policy, parallel_policy,
        result, local_state, num_events, random_number_generator, permutation,
        state_integer_datatype, real_datatype, communicator, environment);
    }

    template <
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator>
    inline void generate_events(
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator& random_number_generator,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      ::ket::mpi::generate_events(
        ::ket::mpi::utility::policy::make_general_mpi(),
        ::ket::utility::policy::make_sequential(),
        result, local_state, num_events, random_number_generator, permutation,
        communicator, environment);
    }

    template <
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator,
      typename DerivedDatatype1, typename DerivedDatatype2>
    inline void generate_events(
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator& random_number_generator,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::datatype_base<DerivedDatatype1> const& state_integer_datatype,
      yampi::datatype_base<DerivedDatatype2> const& real_datatype,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      ::ket::mpi::generate_events(
        ::ket::mpi::utility::policy::make_general_mpi(),
        ::ket::utility::policy::make_sequential(),
        result, local_state, num_events, random_number_generator, permutation,
        state_integer_datatype, real_datatype, communicator, environment);
    }

    template <
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator>
    inline void generate_events(
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator const&,
      typename RandomNumberGenerator::result_type const seed,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      RandomNumberGenerator random_number_generator(seed);
      ::ket::mpi::generate_events(
        result, local_state, num_events, random_number_generator, permutation,
        communicator, environment);
    }

    template <
      typename ResultAllocator,
      typename LocalState, typename RandomNumberGenerator,
      typename StateInteger, typename BitInteger, typename Allocator,
      typename DerivedDatatype1, typename DerivedDatatype2>
    inline void generate_events(
      std::vector<StateInteger, ResultAllocator>& result,
      LocalState& local_state,
      int const num_events,
      RandomNumberGenerator const&,
      typename RandomNumberGenerator::result_type const seed,
      ::ket::mpi::qubit_permutation<
        StateInteger, BitInteger, Allocator>& permutation,
      yampi::datatype_base<DerivedDatatype1> const& state_integer_datatype,
      yampi::datatype_base<DerivedDatatype2> const& real_datatype,
      yampi::communicator const& communicator,
      yampi::environment const& environment)
    {
      RandomNumberGenerator random_number_generator(seed);
      ::ket::mpi::generate_events(
        result, local_state, num_events, random_number_generator, permutation,
        state_integer_datatype, real_datatype, communicator, environment);
    }
  } // namespace mpi
} // namespace ket


#endif

