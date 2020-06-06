//
//  SettingVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class SettingVC: UIViewController {

    @IBOutlet weak var switchActivePin: UISwitch!
    
    var alert: UIAlertController?
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "SETTING")
        tabBarController?.navigationItem.titleView = titleView
        
        if let isActivePin = UserDefaults.standard.value(forKey: "IsActivePin") {
            switchActivePin.isOn = isActivePin as! Bool
        } else {
            switchActivePin.isOn = true
        }
    }
    
    @objc func textFieldDidChange(_ textField: UITextField) {
        guard let alert = alert else {
            return
        }
        
        if (alert.textFields![0].text!.lengthOfBytes(using: .utf8) > 0 &&
            alert.textFields![0].text! == alert.textFields![1].text!) {
            alert.actions[1].isEnabled = true
        } else {
            alert.actions[1].isEnabled = false
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    
    @IBAction func changedValueActivePin(_ sender: Any) {
        let isActivePin = switchActivePin.isOn
        
        UserDefaults.standard.setValue(isActivePin, forKey: "IsActivePin")
        UserDefaults.standard.synchronize()
    }
    
    
    @IBAction func BtnChangePinTapped(_ sender: Any) {
        alert = UIAlertController(title: nil, message: "Change Your Pin", preferredStyle: .alert)
        alert!.addTextField { (textField) in
            textField.placeholder = "Enter your pin"
            textField.keyboardType = .numberPad
            textField.addTarget(self, action: #selector(self.textFieldDidChange(_:)), for: .editingChanged)
            
        }
        alert!.addTextField { (textField) in
            textField.placeholder = "Confirm your pin"
            textField.keyboardType = .numberPad
            textField.addTarget(self, action: #selector(self.textFieldDidChange(_:)), for: .editingChanged)
        }
        
        alert!.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        
        let action = UIAlertAction(title: "OK", style: .default) { (action) in
            // for change pin
            let text = self.alert!.textFields![0].text!
            UserDefaults.standard.set(text, forKey: "Pin")
            UserDefaults.standard.synchronize()
            
        }
        action.isEnabled = false
        alert!.addAction(action)
        present(alert!, animated: true, completion: nil)
    }
    
    
    @IBAction func btnRepeatTestTapped(_ sender: Any) {
    }
    
    
    @IBAction func btnTestProcedureTapped(_ sender: Any) {
        performSegue(withIdentifier: "segueTest", sender: nil)
    }
    
    
    @IBAction func btnSupportTapped(_ sender: Any) {
        let actionSheet = UIAlertController(title: nil, message: "Please contact us", preferredStyle: .actionSheet)
        
        let action1 = UIAlertAction(title: "info@sparkdiagnostics.com", style: .default, handler: { (action) in
            
        })
        action1.actionImage = UIImage(named: "icoEmail")
        actionSheet.addAction(action1)
        
        let action2 = UIAlertAction(title: "+1 415 326 4177", style: .default, handler: { (action) in
            
        })
        action2.actionImage = UIImage(named: "icoPhone")
        actionSheet.addAction(action2)
        
        actionSheet.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        
        present(actionSheet, animated: true, completion: nil)
    }
}
