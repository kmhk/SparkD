//
//  SparkD-Swift.h
//  SparkD
//
//  Created by Pritesh Patel on 6/8/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

#ifndef SparkD_Swift_h
#define SparkD_Swift_h


#endif /* SparkD_Swift_h */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"

#if !defined(__has_include)
# define __has_include(x) 0
#endif
#if !defined(__has_attribute)
# define __has_attribute(x) 0
#endif
#if !defined(__has_feature)
# define __has_feature(x) 0
#endif
#if !defined(__has_warning)
# define __has_warning(x) 0
#endif

#if __has_include(<swift/objc-prologue.h>)
# include <swift/objc-prologue.h>
#endif

#pragma clang diagnostic ignored "-Wauto-import"
#include <Foundation/Foundation.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(SWIFT_TYPEDEFS)
# define SWIFT_TYPEDEFS 1
# if __has_include(<uchar.h>)
#  include <uchar.h>
# elif !defined(__cplusplus)
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
# endif
typedef float swift_float2  __attribute__((__ext_vector_type__(2)));
typedef float swift_float3  __attribute__((__ext_vector_type__(3)));
typedef float swift_float4  __attribute__((__ext_vector_type__(4)));
typedef double swift_double2  __attribute__((__ext_vector_type__(2)));
typedef double swift_double3  __attribute__((__ext_vector_type__(3)));
typedef double swift_double4  __attribute__((__ext_vector_type__(4)));
typedef int swift_int2  __attribute__((__ext_vector_type__(2)));
typedef int swift_int3  __attribute__((__ext_vector_type__(3)));
typedef int swift_int4  __attribute__((__ext_vector_type__(4)));
typedef unsigned int swift_uint2  __attribute__((__ext_vector_type__(2)));
typedef unsigned int swift_uint3  __attribute__((__ext_vector_type__(3)));
typedef unsigned int swift_uint4  __attribute__((__ext_vector_type__(4)));
#endif

#if !defined(SWIFT_PASTE)
# define SWIFT_PASTE_HELPER(x, y) x##y
# define SWIFT_PASTE(x, y) SWIFT_PASTE_HELPER(x, y)
#endif
#if !defined(SWIFT_METATYPE)
# define SWIFT_METATYPE(X) Class
#endif
#if !defined(SWIFT_CLASS_PROPERTY)
# if __has_feature(objc_class_property)
#  define SWIFT_CLASS_PROPERTY(...) __VA_ARGS__
# else
#  define SWIFT_CLASS_PROPERTY(...)
# endif
#endif

#if __has_attribute(objc_runtime_name)
# define SWIFT_RUNTIME_NAME(X) __attribute__((objc_runtime_name(X)))
#else
# define SWIFT_RUNTIME_NAME(X)
#endif
#if __has_attribute(swift_name)
# define SWIFT_COMPILE_NAME(X) __attribute__((swift_name(X)))
#else
# define SWIFT_COMPILE_NAME(X)
#endif
#if __has_attribute(objc_method_family)
# define SWIFT_METHOD_FAMILY(X) __attribute__((objc_method_family(X)))
#else
# define SWIFT_METHOD_FAMILY(X)
#endif
#if __has_attribute(noescape)
# define SWIFT_NOESCAPE __attribute__((noescape))
#else
# define SWIFT_NOESCAPE
#endif
#if __has_attribute(ns_consumed)
# define SWIFT_RELEASES_ARGUMENT __attribute__((ns_consumed))
#else
# define SWIFT_RELEASES_ARGUMENT
#endif
#if __has_attribute(warn_unused_result)
# define SWIFT_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define SWIFT_WARN_UNUSED_RESULT
#endif
#if __has_attribute(noreturn)
# define SWIFT_NORETURN __attribute__((noreturn))
#else
# define SWIFT_NORETURN
#endif
#if !defined(SWIFT_CLASS_EXTRA)
# define SWIFT_CLASS_EXTRA
#endif
#if !defined(SWIFT_PROTOCOL_EXTRA)
# define SWIFT_PROTOCOL_EXTRA
#endif
#if !defined(SWIFT_ENUM_EXTRA)
# define SWIFT_ENUM_EXTRA
#endif
#if !defined(SWIFT_CLASS)
# if __has_attribute(objc_subclassing_restricted)
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# else
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# endif
#endif
#if !defined(SWIFT_RESILIENT_CLASS)
# if __has_attribute(objc_class_stub)
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME) __attribute__((objc_class_stub))
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_class_stub)) SWIFT_CLASS_NAMED(SWIFT_NAME)
# else
#  define SWIFT_RESILIENT_CLASS(SWIFT_NAME) SWIFT_CLASS(SWIFT_NAME)
#  define SWIFT_RESILIENT_CLASS_NAMED(SWIFT_NAME) SWIFT_CLASS_NAMED(SWIFT_NAME)
# endif
#endif

#if !defined(SWIFT_PROTOCOL)
# define SWIFT_PROTOCOL(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
# define SWIFT_PROTOCOL_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
#endif

#if !defined(SWIFT_EXTENSION)
# define SWIFT_EXTENSION(M) SWIFT_PASTE(M##_Swift_, __LINE__)
#endif

#if !defined(OBJC_DESIGNATED_INITIALIZER)
# if __has_attribute(objc_designated_initializer)
#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
# else
#  define OBJC_DESIGNATED_INITIALIZER
# endif
#endif
#if !defined(SWIFT_ENUM_ATTR)
# if defined(__has_attribute) && __has_attribute(enum_extensibility)
#  define SWIFT_ENUM_ATTR(_extensibility) __attribute__((enum_extensibility(_extensibility)))
# else
#  define SWIFT_ENUM_ATTR(_extensibility)
# endif
#endif
#if !defined(SWIFT_ENUM)
# define SWIFT_ENUM(_type, _name, _extensibility) enum _name : _type _name; enum SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# if __has_feature(generalized_swift_name)
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) enum _name : _type _name SWIFT_COMPILE_NAME(SWIFT_NAME); enum SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_ENUM_ATTR(_extensibility) SWIFT_ENUM_EXTRA _name : _type
# else
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME, _extensibility) SWIFT_ENUM(_type, _name, _extensibility)
# endif
#endif
#if !defined(SWIFT_UNAVAILABLE)
# define SWIFT_UNAVAILABLE __attribute__((unavailable))
#endif
#if !defined(SWIFT_UNAVAILABLE_MSG)
# define SWIFT_UNAVAILABLE_MSG(msg) __attribute__((unavailable(msg)))
#endif
#if !defined(SWIFT_AVAILABILITY)
# define SWIFT_AVAILABILITY(plat, ...) __attribute__((availability(plat, __VA_ARGS__)))
#endif
#if !defined(SWIFT_WEAK_IMPORT)
# define SWIFT_WEAK_IMPORT __attribute__((weak_import))
#endif
#if !defined(SWIFT_DEPRECATED)
# define SWIFT_DEPRECATED __attribute__((deprecated))
#endif
#if !defined(SWIFT_DEPRECATED_MSG)
# define SWIFT_DEPRECATED_MSG(...) __attribute__((deprecated(__VA_ARGS__)))
#endif
#if __has_feature(attribute_diagnose_if_objc)
# define SWIFT_DEPRECATED_OBJC(Msg) __attribute__((diagnose_if(1, Msg, "warning")))
#else
# define SWIFT_DEPRECATED_OBJC(Msg) SWIFT_DEPRECATED_MSG(Msg)
#endif
#if !defined(IBSegueAction)
# define IBSegueAction
#endif
#if __has_feature(modules)
#if __has_warning("-Watimport-in-framework-header")
#pragma clang diagnostic ignored "-Watimport-in-framework-header"
#endif
@import CoreGraphics;
@import Foundation;
@import ObjectiveC;
@import QuartzCore;
@import SCRecorder;
@import TOPasscodeViewController;
@import UIKit;
#endif

#pragma clang diagnostic ignored "-Wproperty-attribute-mismatch"
#pragma clang diagnostic ignored "-Wduplicate-method-arg"
#if __has_warning("-Wpragma-clang-attribute")
# pragma clang diagnostic ignored "-Wpragma-clang-attribute"
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wnullability"

#if __has_attribute(external_source_symbol)
# pragma push_macro("any")
# undef any
# pragma clang attribute push(__attribute__((external_source_symbol(language="Swift", defined_in="SparkD",generated_declaration))), apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))
# pragma pop_macro("any")
#endif

@class UITextField;
@class NSBundle;
@class NSCoder;

SWIFT_CLASS("_TtC6SparkD18AddTimerController")
@interface AddTimerController : UIAlertController
- (void)viewDidLoad;
- (void)viewDidLayoutSubviews;
- (void)textDidChangeWithTextField:(UITextField * _Nonnull)textField;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class UIApplication;
@class UISceneSession;
@class UISceneConnectionOptions;
@class UISceneConfiguration;

SWIFT_CLASS("_TtC6SparkD11AppDelegate")
@interface AppDelegate : UIResponder <UIApplicationDelegate>
- (BOOL)application:(UIApplication * _Nonnull)application didFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey, id> * _Nullable)launchOptions SWIFT_WARN_UNUSED_RESULT;
- (UISceneConfiguration * _Nonnull)application:(UIApplication * _Nonnull)application configurationForConnectingSceneSession:(UISceneSession * _Nonnull)connectingSceneSession options:(UISceneConnectionOptions * _Nonnull)options SWIFT_WARN_UNUSED_RESULT SWIFT_AVAILABILITY(ios,introduced=13.0);
- (void)application:(UIApplication * _Nonnull)application didDiscardSceneSessions:(NSSet<UISceneSession *> * _Nonnull)sceneSessions SWIFT_AVAILABILITY(ios,introduced=13.0);
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
@end




SWIFT_CLASS("_TtC6SparkD12GlobalHelper")
@interface GlobalHelper : NSObject
+ (void)fillWithSolver1:(NSArray<NSNumber *> * _Nonnull)solver1 solver2:(NSArray<NSNumber *> * _Nonnull)solver2;
+ (void)fillWithSolver3:(double)solver3 solver4:(double)solver4;
+ (void)fillWithTestT:(NSInteger)testT controlC:(NSInteger)controlC betweenCT:(NSInteger)betweenCT;
+ (void)fillWithMainPatternXYWH:(NSArray<NSArray<NSNumber *> *> * _Nonnull)mainPatternXYWH refPatternXYWH:(NSArray<NSArray<NSNumber *> *> * _Nonnull)refPatternXYWH;
+ (void)fillWithRefPatternLocation:(NSDictionary<NSString *, NSNumber *> * _Nonnull)refPatternLocation;
+ (NSInteger)getCurrentSelectedStripe SWIFT_WARN_UNUSED_RESULT;
+ (NSInteger)getCurrentnTestStrip SWIFT_WARN_UNUSED_RESULT;
+ (NSArray<NSNumber *> * _Nonnull)getxValuesStripWithNPos:(NSInteger)nPos SWIFT_WARN_UNUSED_RESULT;
+ (NSArray<NSNumber *> * _Nonnull)getxValuesOrderWithNPos:(NSInteger)nPos SWIFT_WARN_UNUSED_RESULT;
+ (NSArray<NSNumber *> * _Nonnull)getCurrentnPatternStrip SWIFT_WARN_UNUSED_RESULT;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
@end

@class UIView;

SWIFT_CLASS("_TtC6SparkD6HomeVC")
@interface HomeVC : UIViewController
@property (nonatomic, weak) IBOutlet UIView * _Null_unspecified timerContainer;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (IBAction)btnStartTestTapped:(id _Nonnull)sender;
- (IBAction)btnTimerTapped:(id _Nonnull)sender;
- (IBAction)btnScanTapped:(id _Nonnull)sender;
- (IBAction)btnRecentTimerViewAllTapped:(id _Nonnull)sender;
- (IBAction)btnLastReportViewAllTapped:(id _Nonnull)sender;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


@class UIButton;
@class UILabel;

SWIFT_CLASS("_TtC6SparkD6LogoVC")
@interface LogoVC : UIViewController
@property (nonatomic, weak) IBOutlet UIButton * _Null_unspecified btnStartTest;
@property (nonatomic, weak) IBOutlet UIButton * _Null_unspecified btnPin;
@property (nonatomic, weak) IBOutlet UILabel * _Null_unspecified lblNote;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (IBAction)btnStartTest:(id _Nonnull)sender;
- (IBAction)btnPin:(id _Nonnull)sender;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class TOPasscodeViewController;

@interface LogoVC (SWIFT_EXTENSION(SparkD)) <UIPageViewControllerDelegate>
- (void)didInputCorrectPasscodeInPasscodeViewController:(TOPasscodeViewController * _Nonnull)passcodeViewController;
- (void)didTapCancelInPasscodeViewController:(TOPasscodeViewController * _Nonnull)passcodeViewController;
- (BOOL)passcodeViewController:(TOPasscodeViewController * _Nonnull)passcodeViewController isCorrectCode:(NSString * _Nonnull)code SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6SparkD9MainNavVC")
@interface MainNavVC : UINavigationController
- (void)viewDidLoad;
- (nonnull instancetype)initWithNavigationBarClass:(Class _Nullable)navigationBarClass toolbarClass:(Class _Nullable)toolbarClass OBJC_DESIGNATED_INITIALIZER SWIFT_AVAILABILITY(ios,introduced=5.0);
- (nonnull instancetype)initWithRootViewController:(UIViewController * _Nonnull)rootViewController OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6SparkD8ReportVC")
@interface ReportVC : UIViewController
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6SparkD15RoundShadowView")
@interface RoundShadowView : UIView
- (void)awakeFromNib;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6SparkD6ScanVC")
@interface ScanVC : UIViewController
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class UIImagePickerController;

@interface ScanVC (SWIFT_EXTENSION(SparkD)) <UIImagePickerControllerDelegate, UINavigationControllerDelegate>
- (void)imagePickerController:(UIImagePickerController * _Nonnull)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey, id> * _Nonnull)info;
- (void)imagePickerControllerDidCancel:(UIImagePickerController * _Nonnull)picker;
@end

@class UIWindow;
@class UIScene;

SWIFT_CLASS("_TtC6SparkD13SceneDelegate") SWIFT_AVAILABILITY(ios,introduced=13.0)
@interface SceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (nonatomic, strong) UIWindow * _Nullable window;
- (void)scene:(UIScene * _Nonnull)scene willConnectToSession:(UISceneSession * _Nonnull)session options:(UISceneConnectionOptions * _Nonnull)connectionOptions;
- (void)sceneDidDisconnect:(UIScene * _Nonnull)scene;
- (void)sceneDidBecomeActive:(UIScene * _Nonnull)scene;
- (void)sceneWillResignActive:(UIScene * _Nonnull)scene;
- (void)sceneWillEnterForeground:(UIScene * _Nonnull)scene;
- (void)sceneDidEnterBackground:(UIScene * _Nonnull)scene;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
@end

@class UISwitch;

SWIFT_CLASS("_TtC6SparkD9SettingVC")
@interface SettingVC : UIViewController
@property (nonatomic, weak) IBOutlet UISwitch * _Null_unspecified switchActivePin;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (void)textFieldDidChange:(UITextField * _Nonnull)textField;
- (IBAction)changedValueActivePin:(id _Nonnull)sender;
- (IBAction)BtnChangePinTapped:(id _Nonnull)sender;
- (IBAction)btnRepeatTestTapped:(id _Nonnull)sender;
- (IBAction)btnTestProcedureTapped:(id _Nonnull)sender;
- (IBAction)btnSupportTapped:(id _Nonnull)sender;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class UIImageView;

SWIFT_CLASS("_TtC6SparkD22TPDetailCollectionCell")
@interface TPDetailCollectionCell : UICollectionViewCell
@property (nonatomic, weak) IBOutlet UIView * _Null_unspecified containerView;
@property (nonatomic, weak) IBOutlet UIImageView * _Null_unspecified imageDetail;
@property (nonatomic, weak) IBOutlet UILabel * _Null_unspecified lblDescription;
- (void)layoutSubviews;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


SWIFT_CLASS("_TtC6SparkD31TestProcedureCollectionViewCell")
@interface TestProcedureCollectionViewCell : UICollectionViewCell
@property (nonatomic, weak) IBOutlet UIView * _Null_unspecified containerView;
@property (nonatomic, weak) IBOutlet UIImageView * _Null_unspecified imageView;
@property (nonatomic, weak) IBOutlet UILabel * _Null_unspecified title;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class UIPageControl;
@class UICollectionView;

SWIFT_CLASS("_TtC6SparkD19TestProcedurePageVC")
@interface TestProcedurePageVC : UIViewController
@property (nonatomic, weak) IBOutlet UIButton * _Null_unspecified btnSkip;
@property (nonatomic, weak) IBOutlet UIButton * _Null_unspecified btnNext;
@property (nonatomic, weak) IBOutlet UIPageControl * _Null_unspecified pageControl;
@property (nonatomic, weak) IBOutlet UICollectionView * _Null_unspecified collectionView;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (IBAction)btnSkipTapped:(id _Nonnull)sender;
- (IBAction)btnNextTapped:(id _Nonnull)sender;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end

@class UICollectionViewLayout;
@class UIScrollView;

@interface TestProcedurePageVC (SWIFT_EXTENSION(SparkD)) <UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
- (NSInteger)collectionView:(UICollectionView * _Nonnull)collectionView numberOfItemsInSection:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (UICollectionViewCell * _Nonnull)collectionView:(UICollectionView * _Nonnull)collectionView cellForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (CGSize)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (UIEdgeInsets)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout insetForSectionAtIndex:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (CGFloat)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout minimumLineSpacingForSectionAtIndex:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (CGFloat)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (void)scrollViewDidScroll:(UIScrollView * _Nonnull)scrollView;
@end

@class UIStoryboardSegue;

SWIFT_CLASS("_TtC6SparkD15TestProcedureVC")
@interface TestProcedureVC : UIViewController
@property (nonatomic, weak) IBOutlet UICollectionView * _Null_unspecified collectionView;
@property (nonatomic, weak) IBOutlet UIButton * _Null_unspecified btnStart;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (void)prepareForSegue:(UIStoryboardSegue * _Nonnull)segue sender:(id _Nullable)sender;
- (void)btnBack:(id _Nonnull)sender;
- (IBAction)btnStartTapped:(id _Nonnull)sender;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


@interface TestProcedureVC (SWIFT_EXTENSION(SparkD)) <UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
- (NSInteger)collectionView:(UICollectionView * _Nonnull)collectionView numberOfItemsInSection:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (UICollectionViewCell * _Nonnull)collectionView:(UICollectionView * _Nonnull)collectionView cellForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (CGSize)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (UIEdgeInsets)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout insetForSectionAtIndex:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (void)collectionView:(UICollectionView * _Nonnull)collectionView didSelectItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath;
@end


SWIFT_CLASS("_TtC6SparkD7TimerVC")
@interface TimerVC : UIViewController
@property (nonatomic, weak) IBOutlet UICollectionView * _Null_unspecified collectionView;
- (void)viewDidLoad;
- (void)viewWillAppear:(BOOL)animated;
- (nonnull instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)coder OBJC_DESIGNATED_INITIALIZER;
@end


@interface TimerVC (SWIFT_EXTENSION(SparkD)) <UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
- (NSInteger)collectionView:(UICollectionView * _Nonnull)collectionView numberOfItemsInSection:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
- (UICollectionViewCell * _Nonnull)collectionView:(UICollectionView * _Nonnull)collectionView cellForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (CGSize)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath * _Nonnull)indexPath SWIFT_WARN_UNUSED_RESULT;
- (UIEdgeInsets)collectionView:(UICollectionView * _Nonnull)collectionView layout:(UICollectionViewLayout * _Nonnull)collectionViewLayout insetForSectionAtIndex:(NSInteger)section SWIFT_WARN_UNUSED_RESULT;
@end





@class UIColor;
@class UIFont;

/// <h1>UICircularRing</h1>
/// This is the base class of <code>UICircularProgressRing</code> and <code>UICircularTimerRing</code>.
/// You should not instantiate this class, instead use one of the concrete classes provided
/// or subclass and make your own.
/// This is the UIView subclass that creates and handles everything
/// to do with the circular ring.
/// This class has a custom CAShapeLayer (<code>UICircularRingLayer</code>) which
/// handels the drawing and animating of the view
/// <h2>Author</h2>
/// Luis Padron
IB_DESIGNABLE
SWIFT_CLASS("_TtC6SparkD14UICircularRing")
@interface UICircularRing : UIView
/// Whether or not the progress ring should be a full circle.
/// What this means is that the outer ring will always go from 0 - 360 degrees and
/// the inner ring will be calculated accordingly depending on current value.
/// <h2>Important</h2>
/// Default = true
/// When this property is true any value set for <code>endAngle</code> will be ignored.
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable BOOL fullCircle;
/// A toggle for showing or hiding the value label.
/// If false the current value will not be shown.
/// <h2>Important</h2>
/// Default = true
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable BOOL shouldShowValueText;
/// A toggle for showing or hiding the value knob when current value == minimum value.
/// If false the value knob will not be shown when current value == minimum value.
/// <h2>Important</h2>
/// Default = false
/// <h2>Author</h2>
/// Tom Knapen
@property (nonatomic) IBInspectable BOOL shouldDrawMinValueKnob;
/// The start angle for the entire progress ring view.
/// Please note that Cocoa Touch uses a clockwise rotating unit circle.
/// I.e: 90 degrees is at the bottom and 270 degrees is at the top
/// <h2>Important</h2>
/// Default = 0 (degrees)
/// Values should be in degrees (they’re converted to radians internally)
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat startAngle;
/// The end angle for the entire progress ring
/// Please note that Cocoa Touch uses a clockwise rotating unit circle.
/// I.e: 90 degrees is at the bottom and 270 degrees is at the top
/// <h2>Important</h2>
/// Default = 360 (degrees)
/// Values should be in degrees (they’re converted to radians internally)
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat endAngle;
/// The width of the outer ring for the progres bar
/// <h2>Important</h2>
/// Default = 10.0
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat outerRingWidth;
/// The color for the outer ring
/// <h2>Important</h2>
/// Default = UIColor.gray
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic, strong) IBInspectable UIColor * _Nonnull outerRingColor;
/// The width of the inner ring for the progres bar
/// <h2>Important</h2>
/// Default = 5.0
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat innerRingWidth;
/// The color of the inner ring for the progres bar
/// <h2>Important</h2>
/// Default = UIColor.blue
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic, strong) IBInspectable UIColor * _Nonnull innerRingColor;
/// The spacing between the outer ring and inner ring
/// <h2>Important</h2>
/// This only applies when using <code>ringStyle</code> = <code>.inside</code>
/// Default = 1
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat innerRingSpacing;
/// The text color for the value label field
/// <h2>Important</h2>
/// Default = UIColor.black
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic, strong) IBInspectable UIColor * _Nonnull fontColor;
/// The font to be used for the progress indicator.
/// All font attributes are specified here except for font color, which is done
/// using <code>fontColor</code>.
/// <h2>Important</h2>
/// Default = UIFont.systemFont(ofSize: 18)
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic, strong) IBInspectable UIFont * _Nonnull font;
/// The direction the circle is drawn in
/// Example: true -> clockwise
/// <h2>Important</h2>
/// Default = true (draw the circle clockwise)
/// <h2>Author</h2>
/// Pete Walker
@property (nonatomic) IBInspectable BOOL isClockwise;
/// Overrides the default layer with the custom UICircularRingLayer class
SWIFT_CLASS_PROPERTY(@property (nonatomic, class, readonly) Class _Nonnull layerClass;)
+ (Class _Nonnull)layerClass SWIFT_WARN_UNUSED_RESULT;
/// Overriden public init to initialize the layer and view
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
/// Overriden public init to initialize the layer and view
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
/// Overriden because of custom layer drawing in UICircularRingLayer
- (void)drawRect:(CGRect)rect;
@end


SWIFT_CLASS("_TtC6SparkD22UICircularProgressRing")
@interface UICircularProgressRing : UICircularRing
/// The value property for the progress ring.
/// <h2>Important</h2>
/// Default = 0
/// Must be a non-negative value. If this value falls below <code>minValue</code> it will be
/// clamped and set equal to <code>minValue</code>.
/// This cannot be used to get the value while the ring is animating, to get
/// current value while animating use <code>currentValue</code>.
/// The current value of the progress ring after animating, use startProgress(value:)
/// to alter the value with the option to animate and have a completion handler.
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat value;
/// The minimum value for the progress ring. ex: (0) -> 100.
/// <h2>Important</h2>
/// Default = 100
/// Must be a non-negative value, the absolute value is taken when setting this property.
/// The <code>value</code> of the progress ring must NOT fall below <code>minValue</code> if it does the <code>value</code> property is clamped
/// and will be set equal to <code>value</code>, you will receive a warning message in the console.
/// Making this value greater than
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat minValue;
/// The maximum value for the progress ring. ex: 0 -> (100)
/// <h2>Important</h2>
/// Default = 100
/// Must be a non-negative value, the absolute value is taken when setting this property.
/// Unlike the <code>minValue</code> member <code>value</code> can extend beyond <code>maxValue</code>. What happens in this case
/// is the inner ring will do an extra loop through the outer ring, this is not noticible however.
/// <h2>Author</h2>
/// Luis Padron
@property (nonatomic) IBInspectable CGFloat maxValue;
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
@end




@class NSTimer;

@interface UICircularRing (SWIFT_EXTENSION(SparkD))
/// This method is called when the application goes into the background or when the
/// ProgressRing is paused using the pauseProgress method.
/// This is necessary for the animation to properly pick up where it left off.
/// Triggered by UIApplicationWillResignActive.
/// <h2>Author</h2>
/// Nicolai Cornelis
- (void)snapshotAnimation;
/// This method is called when the application comes back into the foreground or
/// when the ProgressRing is resumed using the continueProgress method.
/// This is necessary for the animation to properly pick up where it left off.
/// Triggered by UIApplicationWillEnterForeground.
/// <h2>Author</h2>
/// Nicolai Cornelis
- (void)restoreAnimation;
/// Called when the animation timer is complete
- (void)animationDidCompleteWithTimer:(NSTimer * _Nonnull)timer;
@end

@protocol CAAction;

/// The internal subclass for CAShapeLayer.
/// This is the class that handles all the drawing and animation.
/// This class is not interacted with, instead
/// properties are set in UICircularRing and those are delegated to here.
SWIFT_CLASS("_TtC6SparkD19UICircularRingLayer")
@interface UICircularRingLayer : CAShapeLayer
@property (nonatomic) CGFloat value;
@property (nonatomic) CGFloat minValue;
@property (nonatomic) CGFloat maxValue;
/// the delegate for the value, is notified when value changes
@property (nonatomic, weak) UICircularRing * _Null_unspecified ring;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithLayer:(id _Nonnull)layer OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
/// Overriden for custom drawing.
/// Draws the outer ring, inner ring and value label.
- (void)drawInContext:(CGContextRef _Nonnull)ctx;
/// Watches for changes in the value property, and setNeedsDisplay accordingly
+ (BOOL)needsDisplayForKey:(NSString * _Nonnull)key SWIFT_WARN_UNUSED_RESULT;
/// Creates animation when value property is changed
- (id <CAAction> _Nullable)actionForKey:(NSString * _Nonnull)event SWIFT_WARN_UNUSED_RESULT;
@end


SWIFT_CLASS("_TtC6SparkD19UICircularTimerRing")
@interface UICircularTimerRing : UICircularRing
- (nonnull instancetype)initWithFrame:(CGRect)frame OBJC_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder * _Nonnull)aDecoder OBJC_DESIGNATED_INITIALIZER;
@end







#if __has_attribute(external_source_symbol)
# pragma clang attribute pop
#endif
#pragma clang diagnostic pop
