// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Use trace_analyzer::Query and trace_analyzer::TraceAnalyzer to search for
// specific trace events that were generated by the trace_event.h API.
//
// Basic procedure:
// - Get trace events JSON string from base::trace_event::TraceLog.
// - Create TraceAnalyzer with JSON string.
// - Call TraceAnalyzer::AssociateBeginEndEvents (optional).
// - Call TraceAnalyzer::AssociateEvents (zero or more times).
// - Call TraceAnalyzer::FindEvents with queries to find specific events.
//
// A Query is a boolean expression tree that evaluates to true or false for a
// given trace event. Queries can be combined into a tree using boolean,
// arithmetic and comparison operators that refer to data of an individual trace
// event.
//
// The events are returned as trace_analyzer::TraceEvent objects.
// TraceEvent contains a single trace event's data, as well as a pointer to
// a related trace event. The related trace event is typically the matching end
// of a begin event or the matching begin of an end event.
//
// The following examples use this basic setup code to construct TraceAnalyzer
// with the json trace string retrieved from TraceLog and construct an event
// vector for retrieving events:
//
// TraceAnalyzer analyzer(json_events);
// TraceEventVector events;
//
// EXAMPLE 1: Find events named "my_event".
//
// analyzer.FindEvents(Query(EVENT_NAME) == "my_event", &events);
//
// EXAMPLE 2: Find begin events named "my_event" with duration > 1 second.
//
// Query q = (Query(EVENT_NAME) == Query::String("my_event") &&
//            Query(EVENT_PHASE) == Query::Phase(TRACE_EVENT_PHASE_BEGIN) &&
//            Query(EVENT_DURATION) > Query::Double(1000000.0));
// analyzer.FindEvents(q, &events);
//
// EXAMPLE 3: Associating event pairs across threads.
//
// If the test needs to analyze something that starts and ends on different
// threads, the test needs to use INSTANT events. The typical procedure is to
// specify the same unique ID as a TRACE_EVENT argument on both the start and
// finish INSTANT events. Then use the following procedure to associate those
// events.
//
// Step 1: instrument code with custom begin/end trace events.
//   [Thread 1 tracing code]
//   TRACE_EVENT_INSTANT1("test_latency", "timing1_begin", "id", 3);
//   [Thread 2 tracing code]
//   TRACE_EVENT_INSTANT1("test_latency", "timing1_end", "id", 3);
//
// Step 2: associate these custom begin/end pairs.
//   Query begin(Query(EVENT_NAME) == Query::String("timing1_begin"));
//   Query end(Query(EVENT_NAME) == Query::String("timing1_end"));
//   Query match(Query(EVENT_ARG, "id") == Query(OTHER_ARG, "id"));
//   analyzer.AssociateEvents(begin, end, match);
//
// Step 3: search for "timing1_begin" events with existing other event.
//   Query q = (Query(EVENT_NAME) == Query::String("timing1_begin") &&
//              Query(EVENT_HAS_OTHER));
//   analyzer.FindEvents(q, &events);
//
// Step 4: analyze events, such as checking durations.
//   for (size_t i = 0; i < events.size(); ++i) {
//     double duration;
//     EXPECT_TRUE(events[i].GetAbsTimeToOtherEvent(&duration));
//     EXPECT_LT(duration, 1000000.0/60.0); // expect less than 1/60 second.
//   }


#ifndef BASE_TEST_TRACE_EVENT_ANALYZER_H_
#define BASE_TEST_TRACE_EVENT_ANALYZER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/trace_event/trace_event.h"

namespace base {
class Value;
}

namespace trace_analyzer {
class QueryNode;

// trace_analyzer::TraceEvent is a more convenient form of the
// base::trace_event::TraceEvent class to make tracing-based tests easier to
// write.
struct TraceEvent {
  // ProcessThreadID contains a Process ID and Thread ID.
  struct ProcessThreadID {
    ProcessThreadID() : process_id(0), thread_id(0) {}
    ProcessThreadID(int process_id, int thread_id)
        : process_id(process_id), thread_id(thread_id) {}
    bool operator< (const ProcessThreadID& rhs) const {
      if (process_id != rhs.process_id)
        return process_id < rhs.process_id;
      return thread_id < rhs.thread_id;
    }
    int process_id;
    int thread_id;
  };

  TraceEvent();
  TraceEvent(TraceEvent&& other);
  ~TraceEvent();

  bool SetFromJSON(const base::Value* event_value) WARN_UNUSED_RESULT;

  bool operator< (const TraceEvent& rhs) const {
    return timestamp < rhs.timestamp;
  }

  TraceEvent& operator=(TraceEvent&& rhs);

  bool has_other_event() const { return other_event; }

  // Returns absolute duration in microseconds between this event and other
  // event. Must have already verified that other_event exists by
  // Query(EVENT_HAS_OTHER) or by calling has_other_event().
  double GetAbsTimeToOtherEvent() const;

  // Return the argument value if it exists and it is a string.
  bool GetArgAsString(const std::string& name, std::string* arg) const;
  // Return the argument value if it exists and it is a number.
  bool GetArgAsNumber(const std::string& name, double* arg) const;
  // Return the argument value if it exists.
  bool GetArgAsValue(const std::string& name,
                     std::unique_ptr<base::Value>* arg) const;

  // Check if argument exists and is string.
  bool HasStringArg(const std::string& name) const;
  // Check if argument exists and is number (double, int or bool).
  bool HasNumberArg(const std::string& name) const;
  // Check if argument exists.
  bool HasArg(const std::string& name) const;

  // Get known existing arguments as specific types.
  // Useful when you have already queried the argument with
  // Query(HAS_NUMBER_ARG) or Query(HAS_STRING_ARG).
  std::string GetKnownArgAsString(const std::string& name) const;
  double GetKnownArgAsDouble(const std::string& name) const;
  int GetKnownArgAsInt(const std::string& name) const;
  bool GetKnownArgAsBool(const std::string& name) const;
  std::unique_ptr<base::Value> GetKnownArgAsValue(
      const std::string& name) const;

  // Process ID and Thread ID.
  ProcessThreadID thread;

  // Time since epoch in microseconds.
  // Stored as double to match its JSON representation.
  double timestamp;
  double duration;
  char phase;
  std::string category;
  std::string name;
  std::string id;

  // All numbers and bool values from TraceEvent args are cast to double.
  // bool becomes 1.0 (true) or 0.0 (false).
  std::map<std::string, double> arg_numbers;
  std::map<std::string, std::string> arg_strings;
  std::map<std::string, std::unique_ptr<base::Value>> arg_values;

  // The other event associated with this event (or NULL).
  const TraceEvent* other_event;
};

typedef std::vector<const TraceEvent*> TraceEventVector;

class Query {
 public:
  Query(const Query& query);

  ~Query();

  ////////////////////////////////////////////////////////////////
  // Query literal values

  // Compare with the given string.
  static Query String(const std::string& str);

  // Compare with the given number.
  static Query Double(double num);
  static Query Int(int32_t num);
  static Query Uint(uint32_t num);

  // Compare with the given bool.
  static Query Bool(bool boolean);

  // Compare with the given phase.
  static Query Phase(char phase);

  // Compare with the given string pattern. Only works with == and != operators.
  // Example: Query(EVENT_NAME) == Query::Pattern("MyEvent*")
  static Query Pattern(const std::string& pattern);

  ////////////////////////////////////////////////////////////////
  // Query event members

  static Query EventPid() { return Query(EVENT_PID); }

  static Query EventTid() { return Query(EVENT_TID); }

  // Return the timestamp of the event in microseconds since epoch.
  static Query EventTime() { return Query(EVENT_TIME); }

  // Return the absolute time between event and other event in microseconds.
  // Only works if Query::EventHasOther() == true.
  static Query EventDuration() { return Query(EVENT_DURATION); }

  // Return the duration of a COMPLETE event.
  static Query EventCompleteDuration() {
    return Query(EVENT_COMPLETE_DURATION);
  }

  static Query EventPhase() { return Query(EVENT_PHASE); }

  static Query EventCategory() { return Query(EVENT_CATEGORY); }

  static Query EventName() { return Query(EVENT_NAME); }

  static Query EventId() { return Query(EVENT_ID); }

  static Query EventPidIs(int process_id) {
    return Query(EVENT_PID) == Query::Int(process_id);
  }

  static Query EventTidIs(int thread_id) {
    return Query(EVENT_TID) == Query::Int(thread_id);
  }

  static Query EventThreadIs(const TraceEvent::ProcessThreadID& thread) {
    return EventPidIs(thread.process_id) && EventTidIs(thread.thread_id);
  }

  static Query EventTimeIs(double timestamp) {
    return Query(EVENT_TIME) == Query::Double(timestamp);
  }

  static Query EventDurationIs(double duration) {
    return Query(EVENT_DURATION) == Query::Double(duration);
  }

  static Query EventPhaseIs(char phase) {
    return Query(EVENT_PHASE) == Query::Phase(phase);
  }

  static Query EventCategoryIs(const std::string& category) {
    return Query(EVENT_CATEGORY) == Query::String(category);
  }

  static Query EventNameIs(const std::string& name) {
    return Query(EVENT_NAME) == Query::String(name);
  }

  static Query EventIdIs(const std::string& id) {
    return Query(EVENT_ID) == Query::String(id);
  }

  // Evaluates to true if arg exists and is a string.
  static Query EventHasStringArg(const std::string& arg_name) {
    return Query(EVENT_HAS_STRING_ARG, arg_name);
  }

  // Evaluates to true if arg exists and is a number.
  // Number arguments include types double, int and bool.
  static Query EventHasNumberArg(const std::string& arg_name) {
    return Query(EVENT_HAS_NUMBER_ARG, arg_name);
  }

  // Evaluates to arg value (string or number).
  static Query EventArg(const std::string& arg_name) {
    return Query(EVENT_ARG, arg_name);
  }

  // Return true if associated event exists.
  static Query EventHasOther() { return Query(EVENT_HAS_OTHER); }

  // Access the associated other_event's members:

  static Query OtherPid() { return Query(OTHER_PID); }

  static Query OtherTid() { return Query(OTHER_TID); }

  static Query OtherTime() { return Query(OTHER_TIME); }

  static Query OtherPhase() { return Query(OTHER_PHASE); }

  static Query OtherCategory() { return Query(OTHER_CATEGORY); }

  static Query OtherName() { return Query(OTHER_NAME); }

  static Query OtherId() { return Query(OTHER_ID); }

  static Query OtherPidIs(int process_id) {
    return Query(OTHER_PID) == Query::Int(process_id);
  }

  static Query OtherTidIs(int thread_id) {
    return Query(OTHER_TID) == Query::Int(thread_id);
  }

  static Query OtherThreadIs(const TraceEvent::ProcessThreadID& thread) {
    return OtherPidIs(thread.process_id) && OtherTidIs(thread.thread_id);
  }

  static Query OtherTimeIs(double timestamp) {
    return Query(OTHER_TIME) == Query::Double(timestamp);
  }

  static Query OtherPhaseIs(char phase) {
    return Query(OTHER_PHASE) == Query::Phase(phase);
  }

  static Query OtherCategoryIs(const std::string& category) {
    return Query(OTHER_CATEGORY) == Query::String(category);
  }

  static Query OtherNameIs(const std::string& name) {
    return Query(OTHER_NAME) == Query::String(name);
  }

  static Query OtherIdIs(const std::string& id) {
    return Query(OTHER_ID) == Query::String(id);
  }

  // Evaluates to true if arg exists and is a string.
  static Query OtherHasStringArg(const std::string& arg_name) {
    return Query(OTHER_HAS_STRING_ARG, arg_name);
  }

  // Evaluates to true if arg exists and is a number.
  // Number arguments include types double, int and bool.
  static Query OtherHasNumberArg(const std::string& arg_name) {
    return Query(OTHER_HAS_NUMBER_ARG, arg_name);
  }

  // Evaluates to arg value (string or number).
  static Query OtherArg(const std::string& arg_name) {
    return Query(OTHER_ARG, arg_name);
  }

  ////////////////////////////////////////////////////////////////
  // Common queries:

  // Find BEGIN events that have a corresponding END event.
  static Query MatchBeginWithEnd() {
    return (Query(EVENT_PHASE) == Query::Phase(TRACE_EVENT_PHASE_BEGIN)) &&
           Query(EVENT_HAS_OTHER);
  }

  // Find COMPLETE events.
  static Query MatchComplete() {
    return (Query(EVENT_PHASE) == Query::Phase(TRACE_EVENT_PHASE_COMPLETE));
  }

  // Find ASYNC_BEGIN events that have a corresponding ASYNC_END event.
  static Query MatchAsyncBeginWithNext() {
    return (Query(EVENT_PHASE) ==
            Query::Phase(TRACE_EVENT_PHASE_ASYNC_BEGIN)) &&
           Query(EVENT_HAS_OTHER);
  }

  // Find BEGIN events of given |name| which also have associated END events.
  static Query MatchBeginName(const std::string& name) {
    return (Query(EVENT_NAME) == Query(name)) && MatchBeginWithEnd();
  }

  // Find COMPLETE events of given |name|.
  static Query MatchCompleteName(const std::string& name) {
    return (Query(EVENT_NAME) == Query(name)) && MatchComplete();
  }

  // Match given Process ID and Thread ID.
  static Query MatchThread(const TraceEvent::ProcessThreadID& thread) {
    return (Query(EVENT_PID) == Query::Int(thread.process_id)) &&
           (Query(EVENT_TID) == Query::Int(thread.thread_id));
  }

  // Match event pair that spans multiple threads.
  static Query MatchCrossThread() {
    return (Query(EVENT_PID) != Query(OTHER_PID)) ||
           (Query(EVENT_TID) != Query(OTHER_TID));
  }

  ////////////////////////////////////////////////////////////////
  // Operators:

  // Boolean operators:
  Query operator==(const Query& rhs) const;
  Query operator!=(const Query& rhs) const;
  Query operator< (const Query& rhs) const;
  Query operator<=(const Query& rhs) const;
  Query operator> (const Query& rhs) const;
  Query operator>=(const Query& rhs) const;
  Query operator&&(const Query& rhs) const;
  Query operator||(const Query& rhs) const;
  Query operator!() const;

  // Arithmetic operators:
  // Following operators are applied to double arguments:
  Query operator+(const Query& rhs) const;
  Query operator-(const Query& rhs) const;
  Query operator*(const Query& rhs) const;
  Query operator/(const Query& rhs) const;
  Query operator-() const;
  // Mod operates on int64_t args (doubles are casted to int64_t beforehand):
  Query operator%(const Query& rhs) const;

  // Return true if the given event matches this query tree.
  // This is a recursive method that walks the query tree.
  bool Evaluate(const TraceEvent& event) const;

 private:
  enum TraceEventMember {
    EVENT_INVALID,
    EVENT_PID,
    EVENT_TID,
    EVENT_TIME,
    EVENT_DURATION,
    EVENT_COMPLETE_DURATION,
    EVENT_PHASE,
    EVENT_CATEGORY,
    EVENT_NAME,
    EVENT_ID,
    EVENT_HAS_STRING_ARG,
    EVENT_HAS_NUMBER_ARG,
    EVENT_ARG,
    EVENT_HAS_OTHER,
    OTHER_PID,
    OTHER_TID,
    OTHER_TIME,
    OTHER_PHASE,
    OTHER_CATEGORY,
    OTHER_NAME,
    OTHER_ID,
    OTHER_HAS_STRING_ARG,
    OTHER_HAS_NUMBER_ARG,
    OTHER_ARG,
  };

  enum Operator {
    OP_INVALID,
    // Boolean operators:
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,
    // Arithmetic operators:
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_NEGATE
  };

  enum QueryType {
    QUERY_BOOLEAN_OPERATOR,
    QUERY_ARITHMETIC_OPERATOR,
    QUERY_EVENT_MEMBER,
    QUERY_NUMBER,
    QUERY_STRING
  };

  // Compare with the given member.
  explicit Query(TraceEventMember member);

  // Compare with the given member argument value.
  Query(TraceEventMember member, const std::string& arg_name);

  // Compare with the given string.
  explicit Query(const std::string& str);

  // Compare with the given number.
  explicit Query(double num);

  // Construct a boolean Query that returns (left <binary_op> right).
  Query(const Query& left, const Query& right, Operator binary_op);

  // Construct a boolean Query that returns (<binary_op> left).
  Query(const Query& left, Operator unary_op);

  // Try to compare left_ against right_ based on operator_.
  // If either left or right does not convert to double, false is returned.
  // Otherwise, true is returned and |result| is set to the comparison result.
  bool CompareAsDouble(const TraceEvent& event, bool* result) const;

  // Try to compare left_ against right_ based on operator_.
  // If either left or right does not convert to string, false is returned.
  // Otherwise, true is returned and |result| is set to the comparison result.
  bool CompareAsString(const TraceEvent& event, bool* result) const;

  // Attempt to convert this Query to a double. On success, true is returned
  // and the double value is stored in |num|.
  bool GetAsDouble(const TraceEvent& event, double* num) const;

  // Attempt to convert this Query to a string. On success, true is returned
  // and the string value is stored in |str|.
  bool GetAsString(const TraceEvent& event, std::string* str) const;

  // Evaluate this Query as an arithmetic operator on left_ and right_.
  bool EvaluateArithmeticOperator(const TraceEvent& event,
                                  double* num) const;

  // For QUERY_EVENT_MEMBER Query: attempt to get the double value of the Query.
  bool GetMemberValueAsDouble(const TraceEvent& event, double* num) const;

  // For QUERY_EVENT_MEMBER Query: attempt to get the string value of the Query.
  bool GetMemberValueAsString(const TraceEvent& event, std::string* num) const;

  // Does this Query represent a value?
  bool is_value() const { return type_ != QUERY_BOOLEAN_OPERATOR; }

  bool is_unary_operator() const {
    return operator_ == OP_NOT || operator_ == OP_NEGATE;
  }

  bool is_comparison_operator() const {
    return operator_ != OP_INVALID && operator_ < OP_AND;
  }

  const Query& left() const;
  const Query& right() const;

  QueryType type_;
  Operator operator_;
  scoped_refptr<QueryNode> left_;
  scoped_refptr<QueryNode> right_;
  TraceEventMember member_;
  double number_;
  std::string string_;
  bool is_pattern_;
};

// Implementation detail:
// QueryNode allows Query to store a ref-counted query tree.
class QueryNode : public base::RefCounted<QueryNode> {
 public:
  explicit QueryNode(const Query& query);
  const Query& query() const { return query_; }

 private:
  friend class base::RefCounted<QueryNode>;
  ~QueryNode();

  Query query_;
};

// TraceAnalyzer helps tests search for trace events.
class TraceAnalyzer {
 public:
  ~TraceAnalyzer();

  // Use trace events from JSON string generated by tracing API.
  // Returns non-NULL if the JSON is successfully parsed.
  static TraceAnalyzer* Create(const std::string& json_events)
                               WARN_UNUSED_RESULT;

  void SetIgnoreMetadataEvents(bool ignore) {
    ignore_metadata_events_ = ignore;
  }

  // Associate BEGIN and END events with each other. This allows Query(OTHER_*)
  // to access the associated event and enables Query(EVENT_DURATION).
  // An end event will match the most recent begin event with the same name,
  // category, process ID and thread ID. This matches what is shown in
  // about:tracing. After association, the BEGIN event will point to the
  // matching END event, but the END event will not point to the BEGIN event.
  void AssociateBeginEndEvents();

  // Associate ASYNC_BEGIN, ASYNC_STEP and ASYNC_END events with each other.
  // An ASYNC_END event will match the most recent ASYNC_BEGIN or ASYNC_STEP
  // event with the same name, category, and ID. This creates a singly linked
  // list of ASYNC_BEGIN->ASYNC_STEP...->ASYNC_END.
  void AssociateAsyncBeginEndEvents();

  // AssociateEvents can be used to customize event associations by setting the
  // other_event member of TraceEvent. This should be used to associate two
  // INSTANT events.
  //
  // The assumptions are:
  // - |first| events occur before |second| events.
  // - the closest matching |second| event is the correct match.
  //
  // |first|  - Eligible |first| events match this query.
  // |second| - Eligible |second| events match this query.
  // |match|  - This query is run on the |first| event. The OTHER_* EventMember
  //            queries will point to an eligible |second| event. The query
  //            should evaluate to true if the |first|/|second| pair is a match.
  //
  // When a match is found, the pair will be associated by having the first
  // event's other_event member point to the other. AssociateEvents does not
  // clear previous associations, so it is possible to associate multiple pairs
  // of events by calling AssociateEvents more than once with different queries.
  //
  // NOTE: AssociateEvents will overwrite existing other_event associations if
  // the queries pass for events that already had a previous association.
  //
  // After calling any Find* method, it is not allowed to call AssociateEvents
  // again.
  void AssociateEvents(const Query& first,
                       const Query& second,
                       const Query& match);

  // For each event, copy its arguments to the other_event argument map. If
  // argument name already exists, it will not be overwritten.
  void MergeAssociatedEventArgs();

  // Find all events that match query and replace output vector.
  size_t FindEvents(const Query& query, TraceEventVector* output);

  // Find first event that matches query or NULL if not found.
  const TraceEvent* FindFirstOf(const Query& query);

  // Find last event that matches query or NULL if not found.
  const TraceEvent* FindLastOf(const Query& query);

  const std::string& GetThreadName(const TraceEvent::ProcessThreadID& thread);

 private:
  TraceAnalyzer();

  bool SetEvents(const std::string& json_events) WARN_UNUSED_RESULT;

  // Read metadata (thread names, etc) from events.
  void ParseMetadata();

  std::map<TraceEvent::ProcessThreadID, std::string> thread_names_;
  std::vector<TraceEvent> raw_events_;
  bool ignore_metadata_events_;
  bool allow_assocation_changes_;

  DISALLOW_COPY_AND_ASSIGN(TraceAnalyzer);
};

// Utility functions for TraceEventVector.

struct RateStats {
  double min_us;
  double max_us;
  double mean_us;
  double standard_deviation_us;
};

struct RateStatsOptions {
  RateStatsOptions() : trim_min(0u), trim_max(0u) {}
  // After the times between events are sorted, the number of specified elements
  // will be trimmed before calculating the RateStats. This is useful in cases
  // where extreme outliers are tolerable and should not skew the overall
  // average.
  size_t trim_min;  // Trim this many minimum times.
  size_t trim_max;  // Trim this many maximum times.
};

// Calculate min/max/mean and standard deviation from the times between
// adjacent events.
bool GetRateStats(const TraceEventVector& events,
                  RateStats* stats,
                  const RateStatsOptions* options);

// Starting from |position|, find the first event that matches |query|.
// Returns true if found, false otherwise.
bool FindFirstOf(const TraceEventVector& events,
                 const Query& query,
                 size_t position,
                 size_t* return_index);

// Starting from |position|, find the last event that matches |query|.
// Returns true if found, false otherwise.
bool FindLastOf(const TraceEventVector& events,
                const Query& query,
                size_t position,
                size_t* return_index);

// Find the closest events to |position| in time that match |query|.
// return_second_closest may be NULL. Closeness is determined by comparing
// with the event timestamp.
// Returns true if found, false otherwise. If both return parameters are
// requested, both must be found for a successful result.
bool FindClosest(const TraceEventVector& events,
                 const Query& query,
                 size_t position,
                 size_t* return_closest,
                 size_t* return_second_closest);

// Count matches, inclusive of |begin_position|, exclusive of |end_position|.
size_t CountMatches(const TraceEventVector& events,
                    const Query& query,
                    size_t begin_position,
                    size_t end_position);

// Count all matches.
static inline size_t CountMatches(const TraceEventVector& events,
                                  const Query& query) {
  return CountMatches(events, query, 0u, events.size());
}

}  // namespace trace_analyzer

#endif  // BASE_TEST_TRACE_EVENT_ANALYZER_H_
