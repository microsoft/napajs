#pragma once

#include <chrono>
#include <ctime>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

namespace Catch {

    /// <summary> Reporter for CATCH test framework that outputs QTest compatible XML. </summary>
    class NapaTestReporter : public StreamingReporterBase {
    public:

        /// <summary> Constructs a NapaTestReporter with the supplied config. </summary>
        /// <param name="config"> Config for reporter to use. </param>
        NapaTestReporter(const ReporterConfig& config) : StreamingReporterBase(config), _os(config.stream()) {
            m_reporterPrefs.shouldRedirectStdOut = true;
        }

        virtual ~NapaTestReporter() CATCH_OVERRIDE {
        };

        /// <summary> String description of the function of this reporter. </summary>
        /// <returns> The description. </returns>
        static std::string getDescription() {
            return "Reports test results as an XML document compatible with QTest";
        }

        /// <summary> If run with a spec for test cases, and none match, print out non-matching spec. </summary>
        /// <param name="filter"> The spec. </param>
        virtual void noMatchingTestCases(const std::string& spec) CATCH_OVERRIDE {
            StreamingReporterBase::noMatchingTestCases(spec);
        }

        /// <summary> Called once per invocation of the test binary, when starting the run. </summary>
        /// <param name="testInfo"> Name, etc. </param>
        virtual void testRunStarting(const TestRunInfo& testInfo) CATCH_OVERRIDE {
            _testResults = _document.allocate_node(rapidxml::node_element, "test-results");
            auto nodeName = _document.allocate_string(testInfo.name.c_str());

            _testResults->append_attribute(_document.allocate_attribute("name", nodeName));

            auto decl = _document.allocate_node(rapidxml::node_declaration);
            decl->append_attribute(_document.allocate_attribute("version", "1.0"));
            decl->append_attribute(_document.allocate_attribute("encoding", "utf-8"));
            decl->append_attribute(_document.allocate_attribute("standalone", "yes"));

            _document.append_node(decl);
            _document.append_node(_testResults);
        }

        /// <summary> Called once per group, when starting the group. </summary>
        /// <param name="groupInfo"> Name, etc. </param>
        virtual void testGroupStarting(const GroupInfo& groupInfo) CATCH_OVERRIDE {
            char *nodeName = _document.allocate_string(groupInfo.name.c_str());
            
            _currentGroup = _document.allocate_node(rapidxml::node_element, "test-suite");
            _currentGroup->append_attribute(_document.allocate_attribute("name", nodeName));

            _testResults->append_node(_currentGroup);

            _currentResult = _document.allocate_node(rapidxml::node_element, "results");
            _currentGroup->append_node(_currentResult);
        }

        /// <summary> Called once per test, when starting the test. </summary>
        /// <param name="testInfo"> Name, etc. </param>
        virtual void testCaseStarting(const TestCaseInfo& testInfo) CATCH_OVERRIDE {
            _currentTest = _document.allocate_node(rapidxml::node_element, "test-case");
            char *nodeName = _document.allocate_string(testInfo.name.c_str());

            _currentTest->append_attribute(_document.allocate_attribute("name", nodeName));
            _testCaseTimer.start();
        }

        /// <summary> Called once per assert, when starting the assert. </summary>
        virtual void assertionStarting(const AssertionInfo&) CATCH_OVERRIDE {
        }

        /// <summary> Called once per assert, when ending the assertion. </summary>
        /// <param name="assertionStats"> Assert result, source, line, etc. </param>
        virtual bool assertionEnded(const AssertionStats& assertionStats) CATCH_OVERRIDE {
            const auto& assertionResult = assertionStats.assertionResult;

            if (isOk(assertionResult.getResultType())) {
                return true;
            }

            std::stringstream ss;
            switch( assertionResult.getResultType() ) {
                case ResultWas::ThrewException:
                case ResultWas::FatalErrorCondition:
                    ss << "At " << assertionResult.getSourceInfo().file << ":" << assertionResult.getSourceInfo().line << ", " << assertionResult.getMessage();
                    break;
                case ResultWas::ExpressionFailed:
                    ss << "At " << assertionResult.getSourceInfo().file << ":" << assertionResult.getSourceInfo().line << ", " << assertionResult.getExpression() << " failed with parameters: " << assertionResult.getExpandedExpression();
                    break;
                default:
                    return true;
                    break;
            }
            
            auto cDataMessage = _document.allocate_string(ss.str().c_str());
            auto cData = _document.allocate_node(rapidxml::node_cdata, nullptr, cDataMessage);
            _currentTest->append_node(cData);

            return true;
        }

        /// <summary> Called once per test, when ending the test. </summary>
        /// <param name="testCaseStats"> Total asserts passed, Name, etc. </param>
        virtual void testCaseEnded(const TestCaseStats& testCaseStats) CATCH_OVERRIDE {
            _currentTest->append_attribute(_document.allocate_attribute("executed", "true"));
            _currentTest->append_attribute(_document.allocate_attribute("success", testCaseStats.totals.assertions.allOk() ? "true" : "false"));
            std::stringstream ss;
            ss << _testCaseTimer.getElapsedSeconds();
            auto seconds = _document.allocate_string(ss.str().c_str());
            _currentTest->append_attribute(_document.allocate_attribute("time", seconds));

            _currentResult->append_node(_currentTest);
        }

        /// <summary> Called once per group, when ending the group. </summary>
        /// <param name="testGroupStats"> Total asserts passed, Name, etc. </param>
        virtual void testGroupEnded(const TestGroupStats& testGroupStats) CATCH_OVERRIDE {
            _currentGroup->append_attribute(_document.allocate_attribute("success", testGroupStats.totals.assertions.failed == 0 ? "true" : "false"));
        }

        /// <summary> Called once per execution of the tests, when exiting. </summary>
        /// <param name="testRunStats"> Total asserts passed, Name, etc. </param>
        virtual void testRunEnded(const TestRunStats& testRunStats) CATCH_OVERRIDE {
            _testResults->append_attribute(_document.allocate_attribute("total", "1"));
            _testResults->append_attribute(_document.allocate_attribute("failures", testRunStats.totals.assertions.failed == 0 ? "0" : "1" ));
            _testResults->append_attribute(_document.allocate_attribute("not-run", "0"));
            
            auto timeAsTimeT = std::time(NULL);
            tm timeTM;

            if (localtime_s(&timeTM, &timeAsTimeT) != 0) {
                _os << _document;
                return;
            }

            char mbstr[100];
            
            std::strftime(mbstr, sizeof(mbstr), "%m/%e/%Y", &timeTM);
            auto date = _document.allocate_string(mbstr);
            _testResults->append_attribute(_document.allocate_attribute("date", date));

            std::strftime(mbstr, sizeof(mbstr), "%H:%M", &timeTM);
            auto time = _document.allocate_string(mbstr);
            _testResults->append_attribute(_document.allocate_attribute("time", time));

            _os << _document;
        }

    private:

        /// <summary> CATCH timer to measure testcase run time. </summary>
        Timer _testCaseTimer;

        /// <summary> The stream to write to, could be cout or file, etc. </summary>
        std::ostream& _os;

        /// <summary> XML structure to contain the results as we go. </summary>
        rapidxml::xml_document<> _document;

        /// <summary> Contains top level node with test stat summary. </summary>
        rapidxml::xml_node<>* _testResults;

        /// <summary> Contains the current test group xml node, owned by _document. </summary>
        rapidxml::xml_node<>* _currentGroup;

        /// <summary> Contains the current result xml node, 1-1 with _currentGroup, owned by _document. </summary>
        rapidxml::xml_node<>* _currentResult;

        /// <summary> Contains the current testxml node, many-1 with _currentResult, owned by _document. </summary>
        rapidxml::xml_node<>* _currentTest;
    };

     INTERNAL_CATCH_REGISTER_REPORTER( "napa", NapaTestReporter )
} // end namespace Catch
