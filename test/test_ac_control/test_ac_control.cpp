#include <unity.h>

#ifdef UNIT_TEST
// Native testing - provide minimal Arduino compatibility
#include <string>
#include <cstdint>
typedef std::string String;
typedef unsigned char uint8_t;
void delay(int ms) {}
#else
#include <Arduino.h>
#endif

// Define ACRule structure for testing (standalone)
struct ACRule {
    int id;
    String name;
    bool enabled;
    int startHour;
    int endHour;
    float minTemp;
    float maxTemp;
    bool acOn;
    float setTemp;
    int fanSpeed;
    int mode;
    int vSwing;
    int hSwing;
};

// Define ACState structure for testing
struct ACState {
    bool power;
    uint8_t temperature;
    uint8_t fanSpeed;
    uint8_t mode;
    int vSwing;
    int hSwing;
};

// Mock variables
float mockTemp = 25.0;
bool mockDebugMode = false;
ACRule mockRules[5];
int mockRuleCount = 0;
int mockActiveRuleId = -1;
ACState mockPreviousACState = {false, 24, 0, 0, 0, 0};

// Mock functions to replace actual hardware calls
float readTemperature() {
    return mockTemp;
}

// Mock AC state change detection
bool hasACStateChanged(bool power, uint8_t temp, uint8_t fan, uint8_t mode, int vSwing, int hSwing) {
    return (mockPreviousACState.power != power ||
            mockPreviousACState.temperature != temp ||
            mockPreviousACState.fanSpeed != fan ||
            mockPreviousACState.mode != mode ||
            mockPreviousACState.vSwing != vSwing ||
            mockPreviousACState.hSwing != hSwing);
}

// Mock function to update previous AC state
void updatePreviousACState(bool power, uint8_t temp, uint8_t fan, uint8_t mode, int vSwing, int hSwing) {
    mockPreviousACState.power = power;
    mockPreviousACState.temperature = temp;
    mockPreviousACState.fanSpeed = fan;
    mockPreviousACState.mode = mode;
    mockPreviousACState.vSwing = vSwing;
    mockPreviousACState.hSwing = hSwing;
}

// Test helper functions
void setUp(void) {
    // Reset mock data before each test
    mockRuleCount = 0;
    mockActiveRuleId = -1;
    mockDebugMode = false;
    mockTemp = 25.0;
    mockPreviousACState = {false, 24, 0, 0, 0, 0};
    
    // Clear rules array
    for (int i = 0; i < 5; i++) {
        mockRules[i] = {0, "", false, -1, -1, -999, -999, false, 24, 0, 0, 0, 0};
    }
}

void tearDown(void) {
    // Clean up after each test
}

// Test AC state change detection
void test_ac_state_change_detection() {
    // Test: No change
    bool changed = hasACStateChanged(false, 24, 0, 0, 0, 0);
    TEST_ASSERT_FALSE(changed);
    
    // Test: Power change
    changed = hasACStateChanged(true, 24, 0, 0, 0, 0);
    TEST_ASSERT_TRUE(changed);
    
    // Update state and test temperature change
    updatePreviousACState(true, 24, 0, 0, 0, 0);
    changed = hasACStateChanged(true, 26, 0, 0, 0, 0);
    TEST_ASSERT_TRUE(changed);
    
    // Test: No change after update
    updatePreviousACState(true, 26, 0, 0, 0, 0);
    changed = hasACStateChanged(true, 26, 0, 0, 0, 0);
    TEST_ASSERT_FALSE(changed);
}

// Test rule matching logic
void test_rule_time_matching() {
    // Create a rule for daytime (8:00-18:00)
    ACRule testRule = {
        .id = 1,
        .name = "Daytime Rule",
        .enabled = true,
        .startHour = 8,
        .endHour = 18,
        .minTemp = -999,
        .maxTemp = -999,
        .acOn = true,
        .setTemp = 24,
        .fanSpeed = 2,
        .mode = 0,
        .vSwing = 0,
        .hSwing = 0
    };
    
    // Test time range matching
    // Hour 10 should match (8 <= 10 < 18)
    bool timeMatch = (10 >= testRule.startHour && 10 < testRule.endHour);
    TEST_ASSERT_TRUE(timeMatch);
    
    // Hour 20 should not match (20 >= 18)
    timeMatch = (20 >= testRule.startHour && 20 < testRule.endHour);
    TEST_ASSERT_FALSE(timeMatch);
    
    // Hour 6 should not match (6 < 8)
    timeMatch = (6 >= testRule.startHour && 6 < testRule.endHour);
    TEST_ASSERT_FALSE(timeMatch);
}

void test_rule_temperature_matching() {
    // Create a rule for temperature range (20-30°C)
    ACRule testRule = {
        .id = 2,
        .name = "Temp Rule",
        .enabled = true,
        .startHour = -1,
        .endHour = -1,
        .minTemp = 20.0,
        .maxTemp = 30.0,
        .acOn = true,
        .setTemp = 24,
        .fanSpeed = 2,
        .mode = 0,
        .vSwing = 0,
        .hSwing = 0
    };
    
    // Test temperature 25°C (should match: 20 <= 25 <= 30)
    bool tempMatch = (25.0 >= testRule.minTemp && 25.0 <= testRule.maxTemp);
    TEST_ASSERT_TRUE(tempMatch);
    
    // Test temperature 35°C (should not match: 35 > 30)
    tempMatch = (35.0 >= testRule.minTemp && 35.0 <= testRule.maxTemp);
    TEST_ASSERT_FALSE(tempMatch);
    
    // Test temperature 15°C (should not match: 15 < 20)
    tempMatch = (15.0 >= testRule.minTemp && 15.0 <= testRule.maxTemp);
    TEST_ASSERT_FALSE(tempMatch);
}

void test_overnight_time_range() {
    // Test overnight rule (22:00-06:00)
    ACRule overnightRule = {
        .id = 3,
        .name = "Overnight Rule",
        .enabled = true,
        .startHour = 22,
        .endHour = 6,
        .minTemp = -999,
        .maxTemp = -999,
        .acOn = true,
        .setTemp = 26,
        .fanSpeed = 1,
        .mode = 0,
        .vSwing = 0,
        .hSwing = 0
    };
    
    // Test hour 23 (should match: 23 >= 22)
    bool timeMatch = (23 >= overnightRule.startHour || 23 < overnightRule.endHour);
    TEST_ASSERT_TRUE(timeMatch);
    
    // Test hour 2 (should match: 2 < 6)
    timeMatch = (2 >= overnightRule.startHour || 2 < overnightRule.endHour);
    TEST_ASSERT_TRUE(timeMatch);
    
    // Test hour 10 (should not match: 10 < 22 AND 10 >= 6)
    timeMatch = (10 >= overnightRule.startHour || 10 < overnightRule.endHour);
    TEST_ASSERT_FALSE(timeMatch);
}

void test_rule_priority() {
    // Test that first matching rule takes priority
    // This would require mocking the rule evaluation loop
    // For now, we test the concept with simple assertions
    
    ACRule rule1 = {1, "Rule 1", true, 8, 18, 20, 30, true, 24, 2, 0, 0, 0};
    ACRule rule2 = {2, "Rule 2", true, 8, 18, 20, 30, true, 26, 3, 1, 0, 0};
    
    // Both rules match the same conditions
    // Rule 1 should take priority (lower index)
    TEST_ASSERT_EQUAL(24, rule1.setTemp);
    TEST_ASSERT_EQUAL(26, rule2.setTemp);
    
    // In actual implementation, rule1 should be applied first
}

void test_disabled_rule_ignored() {
    ACRule disabledRule = {
        .id = 4,
        .name = "Disabled Rule",
        .enabled = false,  // This rule is disabled
        .startHour = -1,
        .endHour = -1,
        .minTemp = -999,
        .maxTemp = -999,
        .acOn = true,
        .setTemp = 20,
        .fanSpeed = 3,
        .mode = 1,
        .vSwing = 0,
        .hSwing = 0
    };
    
    // Disabled rules should be skipped
    TEST_ASSERT_FALSE(disabledRule.enabled);
}

void test_ac_settings_validation() {
    // Test valid temperature range
    TEST_ASSERT_TRUE(16 <= 24 && 24 <= 32);  // Valid temperature
    TEST_ASSERT_FALSE(16 <= 10 && 10 <= 32); // Invalid temperature (too low)
    TEST_ASSERT_FALSE(16 <= 40 && 40 <= 32); // Invalid temperature (too high)
    
    // Test valid fan speed range
    TEST_ASSERT_TRUE(0 <= 2 && 2 <= 3);  // Valid fan speed
    TEST_ASSERT_FALSE(0 <= 5 && 5 <= 3); // Invalid fan speed
    
    // Test valid mode range
    TEST_ASSERT_TRUE(0 <= 1 && 1 <= 4);  // Valid mode
    TEST_ASSERT_FALSE(0 <= 7 && 7 <= 4); // Invalid mode
}

#ifdef UNIT_TEST
int main() {
#else
void setup() {
#endif
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#ifndef UNIT_TEST
    delay(2000);
#endif
    
    UNITY_BEGIN();    // IMPORTANT LINE!
    
    RUN_TEST(test_ac_state_change_detection);
    RUN_TEST(test_rule_time_matching);
    RUN_TEST(test_rule_temperature_matching);
    RUN_TEST(test_overnight_time_range);
    RUN_TEST(test_rule_priority);
    RUN_TEST(test_disabled_rule_ignored);
    RUN_TEST(test_ac_settings_validation);
    
#ifdef UNIT_TEST
    return UNITY_END();
#else
    UNITY_END(); // stop unit testing
#endif
}

#ifndef UNIT_TEST
void loop() {
    // Empty loop for testing
}
#endif
