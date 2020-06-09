//
//  ScanVC.swift
//  SparkD
//
//  Created by com on 6/5/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class ScanVC: UIViewController {
    
    var scanIndex = 3

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        openCamera()
    }
    
    func openCamera() {
        if scanIndex >= 0 {
            scanIndex -= 1
            
            let vc = UIImagePickerController()
            vc.sourceType = .camera
            vc.delegate = self
            present(vc, animated: true, completion: nil)
        }
    }

}

// MARK: -
extension ScanVC: UINavigationControllerDelegate, UIImagePickerControllerDelegate {
    func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {
        picker.dismiss(animated: true, completion: nil)

        guard let image = info[.originalImage] as? UIImage else {
            print("No image found")
            return
        }

        // print out the image size as a test
        print(image.size)
        
        self.openCamera()
    }
    
    func imagePickerControllerDidCancel(_ picker: UIImagePickerController) {
        picker.dismiss(animated: true, completion: nil)
        
        self.openCamera()
    }
}
