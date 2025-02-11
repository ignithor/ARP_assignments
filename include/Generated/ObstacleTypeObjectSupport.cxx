// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file ObstacleTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#include "ObstacleTypeObjectSupport.hpp"

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include "Obstacle.hpp"


using namespace eprosima::fastdds::dds::xtypes;

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_Obstacle_type_identifier(
        TypeIdentifierPair& type_ids_Obstacle)
{

    ReturnCode_t return_code_Obstacle {eprosima::fastdds::dds::RETCODE_OK};
    return_code_Obstacle =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "Obstacle", type_ids_Obstacle);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_Obstacle)
    {
        StructTypeFlag struct_flags_Obstacle = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_Obstacle = "Obstacle";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_Obstacle;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_Obstacle;
        CompleteTypeDetail detail_Obstacle = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_Obstacle, ann_custom_Obstacle, type_name_Obstacle.to_string());
        CompleteStructHeader header_Obstacle;
        header_Obstacle = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_Obstacle);
        CompleteStructMemberSeq member_seq_Obstacle;
        {
            TypeIdentifierPair type_ids_obstacle_x;
            ReturnCode_t return_code_obstacle_x {eprosima::fastdds::dds::RETCODE_OK};
            return_code_obstacle_x =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_array_double_10", type_ids_obstacle_x);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_obstacle_x)
            {
                return_code_obstacle_x =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                    "_double", type_ids_obstacle_x);

                if (eprosima::fastdds::dds::RETCODE_OK != return_code_obstacle_x)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "Array element TypeIdentifier unknown to TypeObjectRegistry.");
                    return;
                }
                bool element_identifier_anonymous_array_double_10_ec {false};
                TypeIdentifier* element_identifier_anonymous_array_double_10 {new TypeIdentifier(TypeObjectUtils::retrieve_complete_type_identifier(type_ids_obstacle_x, element_identifier_anonymous_array_double_10_ec))};
                if (!element_identifier_anonymous_array_double_10_ec)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Array element TypeIdentifier inconsistent.");
                    return;
                }
                EquivalenceKind equiv_kind_anonymous_array_double_10 = EK_COMPLETE;
                if (TK_NONE == type_ids_obstacle_x.type_identifier2()._d())
                {
                    equiv_kind_anonymous_array_double_10 = EK_BOTH;
                }
                CollectionElementFlag element_flags_anonymous_array_double_10 = 0;
                PlainCollectionHeader header_anonymous_array_double_10 = TypeObjectUtils::build_plain_collection_header(equiv_kind_anonymous_array_double_10, element_flags_anonymous_array_double_10);
                {
                    SBoundSeq array_bound_seq;
                        TypeObjectUtils::add_array_dimension(array_bound_seq, static_cast<SBound>(10));

                    PlainArraySElemDefn array_sdefn = TypeObjectUtils::build_plain_array_s_elem_defn(header_anonymous_array_double_10, array_bound_seq,
                                eprosima::fastcdr::external<TypeIdentifier>(element_identifier_anonymous_array_double_10));
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_array_type_identifier(array_sdefn, "anonymous_array_double_10", type_ids_obstacle_x))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_array_double_10 already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_obstacle_x = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_obstacle_x = 0x00000000;
            bool common_obstacle_x_ec {false};
            CommonStructMember common_obstacle_x {TypeObjectUtils::build_common_struct_member(member_id_obstacle_x, member_flags_obstacle_x, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_obstacle_x, common_obstacle_x_ec))};
            if (!common_obstacle_x_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure obstacle_x member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_obstacle_x = "obstacle_x";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_obstacle_x;
            ann_custom_Obstacle.reset();
            CompleteMemberDetail detail_obstacle_x = TypeObjectUtils::build_complete_member_detail(name_obstacle_x, member_ann_builtin_obstacle_x, ann_custom_Obstacle);
            CompleteStructMember member_obstacle_x = TypeObjectUtils::build_complete_struct_member(common_obstacle_x, detail_obstacle_x);
            TypeObjectUtils::add_complete_struct_member(member_seq_Obstacle, member_obstacle_x);
        }
        {
            TypeIdentifierPair type_ids_obstacle_y;
            ReturnCode_t return_code_obstacle_y {eprosima::fastdds::dds::RETCODE_OK};
            return_code_obstacle_y =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_array_double_10", type_ids_obstacle_y);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_obstacle_y)
            {
                return_code_obstacle_y =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                    "_double", type_ids_obstacle_y);

                if (eprosima::fastdds::dds::RETCODE_OK != return_code_obstacle_y)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "Array element TypeIdentifier unknown to TypeObjectRegistry.");
                    return;
                }
                bool element_identifier_anonymous_array_double_10_ec {false};
                TypeIdentifier* element_identifier_anonymous_array_double_10 {new TypeIdentifier(TypeObjectUtils::retrieve_complete_type_identifier(type_ids_obstacle_y, element_identifier_anonymous_array_double_10_ec))};
                if (!element_identifier_anonymous_array_double_10_ec)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Array element TypeIdentifier inconsistent.");
                    return;
                }
                EquivalenceKind equiv_kind_anonymous_array_double_10 = EK_COMPLETE;
                if (TK_NONE == type_ids_obstacle_y.type_identifier2()._d())
                {
                    equiv_kind_anonymous_array_double_10 = EK_BOTH;
                }
                CollectionElementFlag element_flags_anonymous_array_double_10 = 0;
                PlainCollectionHeader header_anonymous_array_double_10 = TypeObjectUtils::build_plain_collection_header(equiv_kind_anonymous_array_double_10, element_flags_anonymous_array_double_10);
                {
                    SBoundSeq array_bound_seq;
                        TypeObjectUtils::add_array_dimension(array_bound_seq, static_cast<SBound>(10));

                    PlainArraySElemDefn array_sdefn = TypeObjectUtils::build_plain_array_s_elem_defn(header_anonymous_array_double_10, array_bound_seq,
                                eprosima::fastcdr::external<TypeIdentifier>(element_identifier_anonymous_array_double_10));
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_array_type_identifier(array_sdefn, "anonymous_array_double_10", type_ids_obstacle_y))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_array_double_10 already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_obstacle_y = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_obstacle_y = 0x00000001;
            bool common_obstacle_y_ec {false};
            CommonStructMember common_obstacle_y {TypeObjectUtils::build_common_struct_member(member_id_obstacle_y, member_flags_obstacle_y, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_obstacle_y, common_obstacle_y_ec))};
            if (!common_obstacle_y_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure obstacle_y member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_obstacle_y = "obstacle_y";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_obstacle_y;
            ann_custom_Obstacle.reset();
            CompleteMemberDetail detail_obstacle_y = TypeObjectUtils::build_complete_member_detail(name_obstacle_y, member_ann_builtin_obstacle_y, ann_custom_Obstacle);
            CompleteStructMember member_obstacle_y = TypeObjectUtils::build_complete_struct_member(common_obstacle_y, detail_obstacle_y);
            TypeObjectUtils::add_complete_struct_member(member_seq_Obstacle, member_obstacle_y);
        }
        CompleteStructType struct_type_Obstacle = TypeObjectUtils::build_complete_struct_type(struct_flags_Obstacle, header_Obstacle, member_seq_Obstacle);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_Obstacle, type_name_Obstacle.to_string(), type_ids_Obstacle))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "Obstacle already registered in TypeObjectRegistry for a different type.");
        }
    }
}

