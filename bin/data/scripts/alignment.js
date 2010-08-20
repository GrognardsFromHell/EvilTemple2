/**
 * Defines the matrix of compatible party alignments.
 */
var CompatibleAlignments = {};

CompatibleAlignments[ Alignment.LawfulGood ] = [Alignment.LawfulGood, Alignment.NeutralGood, Alignment.LawfulNeutral];
CompatibleAlignments[ Alignment.NeutralGood ] = [Alignment.LawfulGood, Alignment.NeutralGood, Alignment.ChaoticGood, Alignment.TrueNeutral];
CompatibleAlignments[ Alignment.ChaoticGood ] = [Alignment.NeutralGood, Alignment.ChaoticGood, Alignment.ChaoticNeutral];
CompatibleAlignments[ Alignment.LawfulNeutral ] = [Alignment.LawfulGood, Alignment.LawfulNeutral, Alignment.TrueNeutral, Alignment.LawfulEvil];
CompatibleAlignments[ Alignment.TrueNeutral ] = [Alignment.NeutralGood, Alignment.LawfulNeutral, Alignment.TrueNeutral, Alignment.ChaoticNeutral, Alignment.NeutralEvil];
CompatibleAlignments[ Alignment.ChaoticNeutral ] = [Alignment.ChaoticGood, Alignment.TrueNeutral, Alignment.ChaoticGood, Alignment.ChaoticEvil];
CompatibleAlignments[ Alignment.LawfulEvil ] = [Alignment.LawfulNeutral, Alignment.LawfulEvil, Alignment.NeutralEvil];
CompatibleAlignments[ Alignment.NeutralEvil ] = [Alignment.TrueNeutral, Alignment.LawfulEvil, Alignment.NeutralEvil, Alignment.ChaoticEvil];
CompatibleAlignments[ Alignment.ChaoticEvil ] = [Alignment.ChaoticNeutral, Alignment.NeutralEvil, Alignment.ChaoticEvil];
