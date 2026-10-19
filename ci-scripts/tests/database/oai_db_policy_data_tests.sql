-- Test fixtures for test_policy_data.py, loaded after
-- build/scripts/oai_db_policy_data.sql. They cover cases the sample data
-- doesn't: several slices and DNNs for one UE, a slice without SD, a stored SD
-- in upper case, a UE policy set, and every optional AmPolicyData attribute.

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
INSERT INTO `UePolicySet` (`ueid`, `policySetId`, `subscCats`, `upsis`, `allowedRouteSelDescs`, `andspInd`, `pei`) VALUES
('208950000000031', 'default', '["standard"]', '["20895-1"]', '{"20895":{"servingPlmn":{"mcc":"208","mnc":"95"},"snssaiRouteSelDescs":[{"snssai":{"sst":222,"sd":"00007b"},"dnnRouteSelDescs":[{"dnn":"default"}]}]}}', 1, 'imeisv-4370816125816151');
