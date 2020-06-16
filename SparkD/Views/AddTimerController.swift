//
//  AddTimerController.swift
//  SparkD
//
//  Created by com on 6/4/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class AddTimerController: UIAlertController {
    
    var imageView: UIImageView?
    
    struct Constants {
        static var paddingAlert: CGFloat = 22
        static var paddingSheet: CGFloat = 11
        static func padding(for style: UIAlertController.Style) -> CGFloat {
            return style == .alert ? Constants.paddingAlert : Constants.paddingSheet
        }
    }
    
    private lazy var lineHeight: CGFloat = {
        let style: UIFont.TextStyle = self.preferredStyle == .alert ? .headline : .callout
        return UIFont.preferredFont(forTextStyle: style).pointSize
    }()

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    var handler: (()->())?
    
    override func viewDidLayoutSubviews() {
        guard let imageView = imageView else {
            super.viewDidLayoutSubviews()
            return
        }
        
        let padding = Constants.padding(for: preferredStyle)
        
        imageView.center.x = view.bounds.width / 2
        imageView.center.y = padding + 2 * lineHeight  / 2
        view.addSubview(imageView)
        super.viewDidLayoutSubviews()
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */
    
    @objc func textDidChange(textField: UITextField) {
        if textField.text!.lengthOfBytes(using: .utf8) > 0 {
            actions[0].isEnabled = true
        } else {
            actions[0].isEnabled = false
        }
    }

    static public func addTimer() -> AddTimerController? {
        // check last time
        if let lastTime = UserDefaults.standard.value(forKey: "last timer") {
            let curDate = Date(timeIntervalSinceNow: 0)
            if curDate.timeIntervalSince(lastTime as! Date) < 120 {
                return nil
            }
        }
        
        let vc = AddTimerController(title: "\n\n", message: "", preferredStyle: .alert)
        
        let image = UIImage(named: "imgAlarm")
        let imgView = UIImageView(frame: CGRect(x: 0, y: 0, width: 40, height: 40))
        imgView.image = image
        vc.imageView = imgView
        
        vc.addTextField { (textField) in
            textField.placeholder = "Timer Name"
            textField.addTarget(vc, action: #selector(vc.textDidChange(textField:)), for: .editingChanged)
        }
        
        let action = UIAlertAction(title: "Add Timer", style: .default) { (action) in
            // add timer
            let curDate = Date(timeIntervalSinceNow: 0)
            let name = vc.textFields![0].text!
            
            let timerDict = ["name": name, "date": curDate] as [String : Any]
            
            var array: [Any] = []
            let timers = UserDefaults.standard.value(forKey: "timers")
            if timers != nil {
                array = timers as! [Any]
            }
            
            array.append(timerDict as Any)
            
            // save to context
            UserDefaults.standard.setValue(curDate, forKey: "last timer")
            UserDefaults.standard.setValue(array, forKey: "timers")
            UserDefaults.standard.synchronize()
            
            // add local notification
            let content = UNMutableNotificationContent()
            content.title = "Alarm"
            content.body = name
            content.sound = UNNotificationSound.default
            content.badge = 1
            content.categoryIdentifier = "scanNotify"
            
            let trigger = UNTimeIntervalNotificationTrigger(timeInterval: 15 * 60, repeats: false)
            
            let request = UNNotificationRequest(identifier: "Local Notification", content: content, trigger: trigger)
            UNUserNotificationCenter.current().add(request) { (error) in
                if let error = error {
                    print("Error on local notification with error: \(error.localizedDescription)")
                }
            }
            
            if vc.handler != nil {
                vc.handler!()
            }
        }
        action.isEnabled = false
        vc.addAction(action)
        
        vc.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        
        return vc
    }
}


extension UIAlertAction {
    
    /// Image to display left of the action title
    var actionImage: UIImage? {
        get {
            if self.responds(to: Selector(Constants.imageKey)) {
                return self.value(forKey: Constants.imageKey) as? UIImage
            }
            return nil
        }
        set {
            if self.responds(to: Selector(Constants.imageKey)) {
                self.setValue(newValue, forKey: Constants.imageKey)
            }
        }
    }
    
    private struct Constants {
        static var imageKey = "image"
    }
}
