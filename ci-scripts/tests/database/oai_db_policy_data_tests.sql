-- Test data for test_policy_data.py, loaded after the schema in
-- build/scripts/oai_db_policy_data.sql (which creates the tables only).
--
-- The first part is the sample data that used to live in that script; the
-- second covers cases it doesn't.

-- ============================================================================
-- SAMPLE DATA
-- ============================================================================
--
-- The following INSERT statements provide sample policy data for testing and
-- development. These entries match UE identifiers (SUPIs) from the existing
-- AuthenticationSubscription and AccessAndMobilitySubscriptionData tables in
-- oai_db_v2.sql.
--
-- UE ID RANGES:
-- - 208950000000031-035: Test UEs with slice sst=222, sd=00007B
-- - 208950000000125-129: Test UEs with slice sst=1, sd=000001
--
-- --------------------------------------------------------

--
-- Sample data for `AccessAndMobilityPolicyData`
--
-- NOTES:
-- - praInfos: Presence reporting areas for location tracking (NULL = not configured)
-- - subscCats: Subscription categories for policy selection (NULL = no special categories)
-- - chfInfo: Charging function addresses (NULL = use default CHF from PCF config)
-- - subscSpendingLimits: Spending limit control disabled (0) for test users
-- - spendLimInfo: Policy counter status (NULL = no active counters)
-- - restriStatus: Restricted access status (NULL = no restrictions)
-- - suppFeat: Supported features negotiated with PCF (NULL = no feature negotiation)
--
-- All sample entries use NULL for policy attributes, meaning default PCF behavior
-- applies. In production, populate these fields based on subscriber policy profile.
--
INSERT INTO `AccessAndMobilityPolicyData` (`ueid`, `praInfos`, `subscCats`, `chfInfo`, `subscSpendingLimits`, `spendLimInfo`, `restriStatus`, `suppFeat`) VALUES
('208950000000031', '{}', '["standard"]', '{}', 0, '{}', '[]', ''),
('208950000000032', '{}', '["standard"]', '{}', 0, '{}', '[]', ''),
('208950000000033', '{}', '["standard"]', '{}', 0, '{}', '[]', ''),
('208950000000034', '{}', '["standard"]', '{}', 0, '{}', '[]', ''),
('208950000000035', '{}', '["standard"]', '{}', 0, '{}', '[]', ''),
('208950000000125', '{}', '["premium"]', '{}', 0, '{}', '[]', ''),
('208950000000126', '{}', '["premium"]', '{}', 0, '{}', '[]', ''),
('208950000000127', '{}', '["premium"]', '{}', 0, '{}', '[]', ''),
('208950000000128', '{}', '["premium"]', '{}', 0, '{}', '[]', ''),
('208950000000129', '{}', '["premium"]', '{}', 0, '{}', '[]', '');

--
-- Sample data for `SessionManagementPolicyData`
--
-- NOTES:
-- - smPolicyData: Complete SmPolicyData JSON structure per TS 29.519 Table 5.4.2.5-1
-- - smPolicySnssaiData: Map with S-NSSAI as key, SmPolicySnssaiData as value
-- - Each SmPolicySnssaiData contains:
--   * snssai: The slice identifier {sst, sd}
--   * smPolicyDnnData: Map with DNN as key, SmPolicyDnnData as value
--     - SmPolicyDnnData contains all per-DNN policy attributes (27 fields from Table 5.4.2.15-1)
-- - Sample entries use minimal policy data (most optional fields omitted)
-- - In production, populate based on subscriber policy profile and SLA
--
-- STRUCTURE:
-- The nested map hierarchy matches the 3GPP standard:
--   SmPolicyData
--   ├── smPolicySnssaiData (map, key = S-NSSAI string)
--   │   └── SmPolicySnssaiData
--   │       ├── snssai: {sst, sd}
--   │       └── smPolicyDnnData (map, key = DNN string)
--   │           └── SmPolicyDnnData: {dnn, subscCats, gbrUl, gbrDl, ...}
--   ├── umDataLimits (map, optional)
--   ├── umData (map, optional)
--   └── suppFeat (string, conditional)
--

INSERT INTO `SessionManagementPolicyData` (`ueid`, `smPolicySnssaiData`, `umDataLimits`, `umData`, `suppFeat`) VALUES
('208950000000031', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","subscCats":["standard"],"gbrUl":"50Mbps","gbrDl":"100Mbps","5qi":9,"arp":{"priorityLevel":8,"preemptCap":"NOT_PREEMPT","preemptVuln":"NOT_PREEMPTABLE"}}},"ueSliceMbr":{"uplink":"1Gbps","downlink":"2Gbps"}}}', '{}', '{}', ''),
('208950000000032', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","subscCats":["standard"],"gbrUl":"50Mbps","gbrDl":"100Mbps","5qi":9,"arp":{"priorityLevel":8,"preemptCap":"NOT_PREEMPT","preemptVuln":"NOT_PREEMPTABLE"}}},"ueSliceMbr":{"uplink":"1Gbps","downlink":"2Gbps"}}}', '{}', '{}', ''),
('208950000000033', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","subscCats":["standard"],"gbrUl":"50Mbps","gbrDl":"100Mbps","5qi":9,"arp":{"priorityLevel":8,"preemptCap":"NOT_PREEMPT","preemptVuln":"NOT_PREEMPTABLE"}}},"ueSliceMbr":{"uplink":"1Gbps","downlink":"2Gbps"}}}', '{}', '{}', ''),
('208950000000034', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","subscCats":["standard"],"gbrUl":"50Mbps","gbrDl":"100Mbps","5qi":9,"arp":{"priorityLevel":8,"preemptCap":"NOT_PREEMPT","preemptVuln":"NOT_PREEMPTABLE"}}},"ueSliceMbr":{"uplink":"1Gbps","downlink":"2Gbps"}}}', '{}', '{}', ''),
('208950000000035', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","subscCats":["standard"],"gbrUl":"50Mbps","gbrDl":"100Mbps","5qi":9,"arp":{"priorityLevel":8,"preemptCap":"NOT_PREEMPT","preemptVuln":"NOT_PREEMPTABLE"}}},"ueSliceMbr":{"uplink":"1Gbps","downlink":"2Gbps"}}}', '{}', '{}', '');

-- --------------------------------------------------------
-- Cases the sample data doesn't cover: several slices and DNNs for one UE, a
-- slice without SD, a stored SD in upper case, a UE policy set, and every
-- optional AmPolicyData attribute.

-- UE 208950000000040: three slices
--   222/00007b -> DNNs default, internet
--   1 (no SD)  -> DNN internet
--   1/00000A   -> DNN ims (upper-case SD, returned as key "1-00000a")
INSERT INTO `SessionManagementPolicyData` (`ueid`, `smPolicySnssaiData`, `umDataLimits`, `umData`, `suppFeat`) VALUES
('208950000000040', '{"{\\"sst\\":222,\\"sd\\":\\"00007b\\"}":{"snssai":{"sst":222,"sd":"00007b"},"smPolicyDnnData":{"default":{"dnn":"default","gbrUl":"20 Mbps","gbrDl":"40 Mbps"},"internet":{"dnn":"internet","gbrUl":"10 Mbps","gbrDl":"20 Mbps"}}},"{\\"sst\\":1}":{"snssai":{"sst":1},"smPolicyDnnData":{"internet":{"dnn":"internet","gbrUl":"5 Mbps","gbrDl":"10 Mbps"}}},"{\\"sst\\":1,\\"sd\\":\\"00000A\\"}":{"snssai":{"sst":1,"sd":"00000A"},"smPolicyDnnData":{"ims":{"dnn":"ims","imsSignallingPrio":true}}}}', NULL, NULL, NULL);

-- UE 208950000000041: AM policy data with every optional attribute set
INSERT INTO `AccessAndMobilityPolicyData` (`ueid`, `praInfos`, `subscCats`, `chfInfo`, `subscSpendingLimits`, `spendLimInfo`, `restriStatus`, `suppFeat`) VALUES
('208950000000041', '{"1":{"praId":"1","presenceState":"IN_AREA"}}', '["gold"]', '{"primaryChfAddress":"http://chf1.example.org:8080","secondaryChfAddress":"http://chf2.example.org:8080"}', 1, '{"pc1":{"policyCounterId":"pc1","currentStatus":"active"}}', '[{"exceptionId":"UNEXPECTED_UE_LOCATION","timeStamp":"2026-01-01T00:00:00Z"}]', '3');

-- UE 208950000000031: a UE policy set (the sample data has none)
INSERT INTO `UePolicySet` (`ueid`, `subscCats`, `upsis`, `allowedRouteSelDescs`, `andspInd`, `pei`) VALUES
('208950000000031', '["standard"]', '["20895-1"]', '{"20895":{"servingPlmn":{"mcc":"208","mnc":"95"},"snssaiRouteSelDescs":[{"snssai":{"sst":222,"sd":"00007b"},"dnnRouteSelDescs":[{"dnn":"default"}]}]}}', 1, 'imeisv-4370816125816151');
